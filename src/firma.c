#include "firma.h"
#include "md5.h"

#include <stdio.h>
#include <string.h>

/*
 * Buffer intermedio donde armamos el "texto a firmar".
 * 1024 alcanza y sobra: el checkpoint 2 limita la PDU a 122 bytes,
 * el usuario y el secreto son cortos, y el IPv4 son 15 chars como máximo.
 */
#define TEXTO_MAX 1024

size_t armar_pdu_auth(char *salida, size_t tam_salida,
                      const char *usuario,
                      const char *ip,
                      const char *secreto)
{
    char texto[TEXTO_MAX];
    char firma[33];

    /* 1) Armar el string que se va a hashear, con el secreto al final */
    int n = snprintf(texto, sizeof texto, "%s|%s|%s", usuario, ip, secreto);
    if (n < 0 || n >= (int)sizeof texto) return 0;

    /* 2) Firmarlo */
    md5(texto, firma);

    /* 3) Armar la PDU final (sin el secreto, con la firma en su lugar) */
    int m = snprintf(salida, tam_salida, "%s|%s|%s", usuario, ip, firma);
    if (m < 0 || m >= (int)tam_salida) return 0;
    return (size_t)m;
}

size_t armar_pdu_reenvio(char *salida, size_t tam_salida,
                         const char *usuario,
                         const char *pdu,
                         const char *parse_code,
                         const char *secreto)
{
    char texto[TEXTO_MAX];
    char firma[33];

    int n = snprintf(texto, sizeof texto, "%s|%s|%s|%s",
                     usuario, pdu, parse_code, secreto);
    if (n < 0 || n >= (int)sizeof texto) return 0;

    md5(texto, firma);

    int m = snprintf(salida, tam_salida, "%s|%s|%s|%s",
                     usuario, pdu, parse_code, firma);
    if (m < 0 || m >= (int)tam_salida) return 0;
    return (size_t)m;
}

int validar_firma(const char *cuerpo,
                  const char *firma_recibida,
                  const char *secreto)
{
    char texto[TEXTO_MAX];
    char firma_esperada[33];

    int n = snprintf(texto, sizeof texto, "%s|%s", cuerpo, secreto);
    if (n < 0 || n >= (int)sizeof texto) return 0;

    md5(texto, firma_esperada);
    return strcmp(firma_esperada, firma_recibida) == 0;
}