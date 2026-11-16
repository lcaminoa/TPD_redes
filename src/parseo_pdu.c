#include "parseo_pdu.h"

#include <stdlib.h> // atoll
#include <string.h> // memchr, memcpy, memmove, strcpy
#include <time.h> // time

/*
Busca el primer \r\n (0x0D 0x0A) en los primeros len bytes de buf
Si lo encuentra devuelve 1 y deja en pos_out donde está el '\r', si no devuelve 0
*/
static int buscar_crlf(const char *buf, size_t len, size_t *pos_out)
{
    for (size_t i = 0; i + 1 < len; i++) {
        if (buf[i] == 0x0D && buf[i + 1] == 0x0A) {
            *pos_out = i;
            return 1;
        }
    }
    return 0;
}

/*
Le asigna el parse_code a una PDU (ya sin el \r\n)
Tenemos <TIMESTAMP>&<MESSAGE>, ej: 1792303547&Temperatura ambiente 11 grados
1. 400 si no hay '&'
2. Timestamp: 407 si tiene mas de 11 chars, 405 si no son todos digitos,
   406 si difiere en mas de 300 seg con la hora actual
3. Message: 409 si está vacio, 410 si tiene mas de 100 chars,
   408 si tiene chars fuera de [0x20..0x7D] o '&' o '|'
4. Si todo pasa: 200
*/
static void clasificar(const char *pdu, size_t len, char parse_code_out[4])
{
    // 1. Separador '&'
    const char *amp = memchr(pdu, '&', len);
    if (!amp) {
        strcpy(parse_code_out, "400");
        return;
    }
    // separamos timestamp y message
    size_t timestamp_len = (size_t)(amp - pdu);
    const char *message = amp + 1;
    size_t message_len = len - timestamp_len - 1;

    // 2. condiciones sobre timestamp
    if (timestamp_len > PDU_TIMESTAMP_MAX) {
        strcpy(parse_code_out, "407");
        return;
    }
    for (size_t i = 0; i < timestamp_len; i++) {
        if (pdu[i] < '0' || pdu[i] > '9') {
            strcpy(parse_code_out, "405");
            return;
        }
    }
    // Lo pasamos a numero para ver la diferencia con la hora actual
    char ts_str[PDU_TIMESTAMP_MAX + 1];
    memcpy(ts_str, pdu, timestamp_len);
    ts_str[timestamp_len] = '\0';
    long long ts = atoll(ts_str);
    long long ahora = (long long)time(NULL);
    long long delta = ahora - ts;
    if (delta < 0) delta = -delta; // puede estar adelantado o atrasado
    if (delta > PDU_DELTA_MAX_SEG) {
        strcpy(parse_code_out, "406");
        return;
    }

    // 3. condiciones sobre msj
    if (message_len == 0) {
        strcpy(parse_code_out, "409");
        return;
    }
    if (message_len > PDU_MESSAGE_MAX) {
        strcpy(parse_code_out, "410");
        return;
    }
    for (size_t i = 0; i < message_len; i++) {
        unsigned char c = (unsigned char)message[i];
        if (c < 0x20 || c > 0x7D || c == 0x26 || c == 0x7C) {
            strcpy(parse_code_out, "408");
            return;
        }
    }

    // todo ok -> no hay errores de parseo
    strcpy(parse_code_out, "200");
}

int pdu_parser_feed(pdu_parser_t *p, const char *chunk, size_t n)
{
    // si lo que ya hay + lo nuevo no entra en el buffer, devolvemos error
    if (p->len + n > PDU_PARSER_BUFFER_SIZE) {
        fprintf(stderr, "Buffer del parser lleno: %zu + %zu > %d\n", p->len, n, PDU_PARSER_BUFFER_SIZE);
        return -1;
    }
    // copiamos el chunk al final de lo que ya teniamos
    memcpy(p->buffer + p->len, chunk, n);
    p->len += n;
    return 0;
}

int pdu_parser_siguiente(pdu_parser_t *p, char pdu_out[PDU_PARSER_BUFFER_SIZE], char parse_code_out[4], int *enviar_out)
{
    size_t pos; // donde empieza el \r\n
    if (!buscar_crlf(p->buffer, p->len, &pos)) {
        return 0; // todavia no llegó una PDU completa
    }
    size_t pdu_len = pos;

    // Copiamos la PDU entera (sin el \r\n), aunque pase los 122 bytes:
    // hay que reenviarla tal cual llegó con su parse_code
    // (si es mas larga, es porque el timestamp o el message se pasan -> 407 o 410)
    memcpy(pdu_out, p->buffer, pdu_len);
    pdu_out[pdu_len] = '\0';

    clasificar(pdu_out, pdu_len, parse_code_out);

    // Solo se reenvia si la PDU no tiene '|' (0x7C)
    *enviar_out = (memchr(pdu_out, '|', pdu_len) == NULL);

    // Sacamos del buffer la PDU + el \r\n y corremos lo que queda al principio
    size_t consumido = pdu_len + 2;
    size_t restante = p->len - consumido;
    if (restante > 0) {
        memmove(p->buffer, p->buffer + consumido, restante);
    }
    p->len = restante;
    return 1;
}
