#include "int_var.h"
#include "crypto.h"

#include <string.h>
#include <time.h>
#include <stdio.h>

/*
Struttura interna in chiaro.
Questa struttura non viene mai esposta all'esterno.
*/
typedef struct
{
    int value;
    uint64_t timestamp;
    char ide[IDE_SIZE];

} internal_var;


/*
Serializza la struttura internal_var in un buffer di byte.
*/
static void serialize_internal(
    const internal_var *src,
    unsigned char *dst)
{
    memcpy(dst, src, sizeof(internal_var));
}


/*
Deserializza un buffer in una struttura internal_var.
*/
static void deserialize_internal(
    const unsigned char *src,
    internal_var *dst)
{
    memcpy(dst, src, sizeof(internal_var));
}


/*
Crea una nuova variabile protetta.
*/
void int_var_init(int_var v, const char *ide)
{
    internal_var temp;

    temp.value = 0;

    temp.timestamp = (uint64_t)time(NULL);

    memset(temp.ide, 0, IDE_SIZE);
    strncpy(temp.ide, ide, IDE_SIZE - 1);

    unsigned char buffer[INTERNAL_VAR_SIZE];

    serialize_internal(&temp, buffer);

    crypto_encrypt(
        buffer,
        INTERNAL_VAR_SIZE,
        v,
        INT_VAR_SIZE
    );
}


/*
Assegna un valore a una variabile cifrata.
*/
void int_var_assign(int_var v, int value)
{
    unsigned char buffer[INTERNAL_VAR_SIZE];

    internal_var temp;

    crypto_decrypt(
        v,
        INT_VAR_SIZE,
        buffer,
        INTERNAL_VAR_SIZE
    );

    deserialize_internal(buffer, &temp);

    temp.value = value;

    temp.timestamp = (uint64_t)time(NULL);

    serialize_internal(&temp, buffer);

    crypto_encrypt(
        buffer,
        INTERNAL_VAR_SIZE,
        v,
        INT_VAR_SIZE
    );
}


/*
Legge il valore della variabile.
*/
int int_var_get(const int_var v)
{
    unsigned char buffer[INTERNAL_VAR_SIZE];

    internal_var temp;

    crypto_decrypt(
        v,
        INT_VAR_SIZE,
        buffer,
        INTERNAL_VAR_SIZE
    );

    deserialize_internal(buffer, &temp);

    return temp.value;
}