#include "parseo_pdu.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ------------------------------------------------------------------ */
/* Helpers internos                                                   */
/* ------------------------------------------------------------------ */

/* Busca la primera aparición de 0x0D 0x0A en los primeros `len` bytes
   de `buf`. Devuelve 1 y escribe la posición del '\r' en *pos_out;
   devuelve 0 si no apareció. */
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
 * Asigna el PARSE_CODE a una PDU candidata (sin CRLF).
 *
 * Orden de validación (importante para los ejemplos del enunciado):
 *   1. 400  si no hay '&' (no se forman los 2 campos).
 *   2. Timestamp:
 *      407  si tiene > 11 chars.
 *      405  si tiene chars fuera de 0x30-0x39.
 *      406  si |actual - timestamp| > 300 s.
 *   3. Message:
 *      409  si está vacío.
 *      410  si tiene > 100 chars.
 *      408  si tiene chars fuera de [0x20..0x7D] o es 0x26/0x7C.
 *   4. Si todo pasa: 200.
 *
 * `pdu` es un C-string (termina en '\0'); `len` es su longitud útil.
 */
static void clasificar(const char *pdu, size_t len, char parse_code_out[4])
{
    /* 1) Separador '&' */
    const char *amp = memchr(pdu, '&', len);
    if (!amp) {
        strcpy(parse_code_out, "400");
        return;
    }

    size_t timestamp_len  = (size_t)(amp - pdu);
    const char *message   = amp + 1;
    size_t message_len    = len - timestamp_len - 1;

    /* 2) Timestamp */
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
    /* Convertimos a long long para chequear antigüedad. */
    char ts_str[PDU_TIMESTAMP_MAX + 1];
    memcpy(ts_str, pdu, timestamp_len);
    ts_str[timestamp_len] = '\0';
    long long ts     = atoll(ts_str);
    long long ahora  = (long long)time(NULL);
    long long delta  = ahora - ts;
    if (delta < 0) delta = -delta;   /* tanto adelanto como atraso */
    if (delta > PDU_DELTA_MAX_SEG) {
        strcpy(parse_code_out, "406");
        return;
    }

    /* 3) Message */
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

    /* 4) Todo OK */
    strcpy(parse_code_out, "200");
}

/* ------------------------------------------------------------------ */
/* API pública                                                        */
/* ------------------------------------------------------------------ */

void pdu_parser_init(pdu_parser_t *p)
{
    p->len = 0;
}

int pdu_parser_feed(pdu_parser_t *p, const char *chunk, size_t n)
{
    if (p->len + n > PDU_PARSER_BUFFER_SIZE) return -1;
    memcpy(p->buffer + p->len, chunk, n);
    p->len += n;
    return 0;
}

int pdu_parser_siguiente(pdu_parser_t *p,
                         char pdu_out[PDU_MAX_LEN + 1],
                         char parse_code_out[4],
                         int *enviar_out)
{
    size_t pos;
    if (!buscar_crlf(p->buffer, p->len, &pos)) {
        return 0;   /* no hay PDU completa aún */
    }

    /* `pos` apunta al '\r'; `pdu_len` es cuánto mide la PDU SIN CRLF. */
    size_t pdu_len = pos;

    /* El enunciado fija el máximo total en 122 bytes (incluyendo CRLF),
       lo que da 120 bytes útiles. Si se excede, la marcamos 400 y
       truncamos lo copiado para no desbordar el buffer de salida. */
    size_t copy_len = (pdu_len > PDU_MAX_LEN) ? PDU_MAX_LEN : pdu_len;
    memcpy(pdu_out, p->buffer, copy_len);
    pdu_out[copy_len] = '\0';

    if (pdu_len > PDU_MAX_LEN) {
        strcpy(parse_code_out, "400");
    } else {
        clasificar(pdu_out, pdu_len, parse_code_out);
    }

    /* El reenvío solo sucede si la PDU no contiene '|' (0x7C). Lo
       chequeamos sobre lo que vamos a mandar (pdu_out). */
    *enviar_out = (memchr(pdu_out, '|', copy_len) == NULL);

    /* Consumir esta PDU + su CRLF del buffer interno. */
    size_t consumido = pdu_len + 2;   /* +2 por \r\n */
    size_t restante  = p->len - consumido;
    if (restante > 0) {
        memmove(p->buffer, p->buffer + consumido, restante);
    }
    p->len = restante;

    return 1;
}