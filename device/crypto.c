#include "crypto.h"

#include <string.h>
#include <stdio.h>

#include <mbedtls/gcm.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>

/*
 Device AES key storage.
 Questa chiave rappresenta la chiave pre-provisionata del dispositivo.
*/
static uint8_t device_key[AES_KEY_SIZE];

/*
 Flag che indica se la chiave è stata inizializzata.
*/
static int key_initialized = 0;

/*
 Context per generazione IV random.
*/
static mbedtls_entropy_context entropy;
static mbedtls_ctr_drbg_context ctr_drbg;

/*
------------------------------------------------------------
crypto_init
------------------------------------------------------------
Inizializza il modulo crittografico.

Operazioni:
1. copia la chiave nel contesto interno
2. inizializza il generatore random per IV
*/
crypto_status_t crypto_init(const uint8_t *key)
{
    if (key == NULL)
        return CRYPTO_ERR_NULL;

    memcpy(device_key, key, AES_KEY_SIZE);

    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    const char *pers = "secure_workflow_crypto";

    if (mbedtls_ctr_drbg_seed(
            &ctr_drbg,
            mbedtls_entropy_func,
            &entropy,
            (const unsigned char *)pers,
            strlen(pers)) != 0)
    {
        return CRYPTO_ERR_INIT;
    }

    key_initialized = 1;

    return CRYPTO_OK;
}

/*
------------------------------------------------------------
crypto_encrypt
------------------------------------------------------------
Cifra un buffer con AES-GCM.

Output layout:

| IV | CIPHERTEXT | TAG |
*/
crypto_status_t crypto_encrypt(
    const uint8_t *plaintext,
    size_t plaintext_len,
    uint8_t *output,
    size_t output_len)
{
    if (!key_initialized)
        return CRYPTO_ERR_INIT;

    if (plaintext == NULL || output == NULL)
        return CRYPTO_ERR_NULL;

    size_t required = AES_IV_SIZE + plaintext_len + AES_TAG_SIZE;

    if (output_len < required)
        return CRYPTO_ERR_ENCRYPT;

    uint8_t iv[AES_IV_SIZE];
    uint8_t tag[AES_TAG_SIZE];

    if (mbedtls_ctr_drbg_random(&ctr_drbg, iv, AES_IV_SIZE) != 0)
        return CRYPTO_ERR_ENCRYPT;

    mbedtls_gcm_context gcm;
    mbedtls_gcm_init(&gcm);

    if (mbedtls_gcm_setkey(
            &gcm,
            MBEDTLS_CIPHER_ID_AES,
            device_key,
            AES_KEY_SIZE * 8) != 0)
    {
        mbedtls_gcm_free(&gcm);
        return CRYPTO_ERR_ENCRYPT;
    }

    uint8_t *ciphertext = output + AES_IV_SIZE;

    if (mbedtls_gcm_crypt_and_tag(
            &gcm,
            MBEDTLS_GCM_ENCRYPT,
            plaintext_len,
            iv,
            AES_IV_SIZE,
            NULL,
            0,
            plaintext,
            ciphertext,
            AES_TAG_SIZE,
            tag) != 0)
    {
        mbedtls_gcm_free(&gcm);
        return CRYPTO_ERR_ENCRYPT;
    }

    memcpy(output, iv, AES_IV_SIZE);

    memcpy(output + AES_IV_SIZE + plaintext_len, tag, AES_TAG_SIZE);

    mbedtls_gcm_free(&gcm);

    return CRYPTO_OK;
}

/*
------------------------------------------------------------
crypto_decrypt
------------------------------------------------------------
Decifra buffer AES-GCM.

Verifica automaticamente il TAG.
*/
crypto_status_t crypto_decrypt(
    const uint8_t *input,
    size_t input_len,
    uint8_t *plaintext,
    size_t plaintext_len)
{
    if (!key_initialized)
        return CRYPTO_ERR_INIT;

    if (input == NULL || plaintext == NULL)
        return CRYPTO_ERR_NULL;

    if (input_len < AES_IV_SIZE + AES_TAG_SIZE)
        return CRYPTO_ERR_DECRYPT;

    size_t ciphertext_len = input_len - AES_IV_SIZE - AES_TAG_SIZE;

    if (plaintext_len < ciphertext_len)
        return CRYPTO_ERR_DECRYPT;

    const uint8_t *iv = input;
    const uint8_t *ciphertext = input + AES_IV_SIZE;
    const uint8_t *tag = input + AES_IV_SIZE + ciphertext_len;

    mbedtls_gcm_context gcm;
    mbedtls_gcm_init(&gcm);

    if (mbedtls_gcm_setkey(
            &gcm,
            MBEDTLS_CIPHER_ID_AES,
            device_key,
            AES_KEY_SIZE * 8) != 0)
    {
        mbedtls_gcm_free(&gcm);
        return CRYPTO_ERR_DECRYPT;
    }

    if (mbedtls_gcm_auth_decrypt(
            &gcm,
            ciphertext_len,
            iv,
            AES_IV_SIZE,
            NULL,
            0,
            tag,
            AES_TAG_SIZE,
            ciphertext,
            plaintext) != 0)
    {
        mbedtls_gcm_free(&gcm);
        return CRYPTO_ERR_AUTH;
    }

    mbedtls_gcm_free(&gcm);

    return CRYPTO_OK;
}