#include "firma.h"
#include "md5.h"

#include <stdio.h> // snprintf
#include <string.h> // strcmp

// Buffer para el texto a firmar. Tiene que entrar una PDU entera aunque sea
// invalida (hasta 8192, el buffer del parser) + usuario, code y secreto
#define TEXTO_MAX (8192 + 512)

size_t armar_pdu_auth(char *salida, size_t tam_salida, const char *usuario, const char *ip, const char *secreto)
{
    char texto[TEXTO_MAX], firma[33];

    // Armamos el string que se va a hashear, con el secreto al final
    int n = snprintf(texto, sizeof texto, "%s|%s|%s", usuario, ip, secreto);
    if (n < 0 || n >= (int)sizeof texto) return 0;

    // firma = md5(usuario|ip|secreto)
    md5(texto, firma);

    // PDU final: usuario|ip|firma
    int m = snprintf(salida, tam_salida, "%s|%s|%s", usuario, ip, firma);
    if (m < 0 || m >= (int)tam_salida) return 0;
    return (size_t)m;
}

size_t armar_pdu_reenvio(char *salida, size_t tam_salida, const char *usuario, const char *pdu,
                         const char *parse_code, const char *secreto)
{
    char texto[TEXTO_MAX], firma[33];

    // Armamos el string que se va a hashear, con el secreto al final
    int n = snprintf(texto, sizeof texto, "%s|%s|%s|%s", usuario, pdu, parse_code, secreto);
    if (n < 0 || n >= (int)sizeof texto) return 0;

    // firma = md5(usuario|pdu|parse_code|secreto)
    md5(texto, firma);

    // PDU final: usuario|pdu|parse_code|firma
    int m = snprintf(salida, tam_salida, "%s|%s|%s|%s", usuario, pdu, parse_code, firma);
    if (m < 0 || m >= (int)tam_salida) return 0;
    return (size_t)m;
}

int validar_firma(const char *cuerpo, const char *firma_recibida, const char *secreto)
{
    char texto[TEXTO_MAX], esperada[33];

    int n = snprintf(texto, sizeof texto, "%s|%s", cuerpo, secreto);
    if (n < 0 || n >= (int)sizeof texto) return 0;

    // Comparamos la firma recibida con la esperada -> para validar al servidor
    md5(texto, esperada);
    return strcmp(esperada, firma_recibida) == 0;
}
