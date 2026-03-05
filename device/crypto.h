#ifndef CRYPTO_H
#define CRYPTO_H

#include <stddef.h>
#include <stdint.h>

/*
 Dimensioni parametri AES-GCM
*/
#define AES_KEY_SIZE 16
#define AES_IV_SIZE 12
#define AES_TAG_SIZE 16

/*
 Codici di errore del modulo crypto
*/
typedef enum {

    CRYPTO_OK = 0,
    CRYPTO_ERR_NULL,
    CRYPTO_ERR_INIT,
    CRYPTO_ERR_ENCRYPT,
    CRYPTO_ERR_DECRYPT,
    CRYPTO_ERR_AUTH

} crypto_status_t;


/*
---------------------------------------------------------
crypto_init

Inizializza il modulo crittografico e carica la chiave
del dispositivo.
---------------------------------------------------------
*/
crypto_status_t crypto_init(const uint8_t *key);


/*
---------------------------------------------------------
crypto_encrypt

Cifra un buffer con AES-GCM.

output layout:

| IV | ciphertext | tag |
---------------------------------------------------------
*/
crypto_status_t crypto_encrypt(
    const uint8_t *plaintext,
    size_t plaintext_len,
    uint8_t *output,
    size_t output_len
);


/*
---------------------------------------------------------
crypto_decrypt

Decifra un buffer cifrato AES-GCM.

Verifica automaticamente il TAG.
---------------------------------------------------------
*/
crypto_status_t crypto_decrypt(
    const uint8_t *input,
    size_t input_len,
    uint8_t *plaintext,
    size_t plaintext_len
);

#endif