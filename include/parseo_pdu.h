#ifndef PARSEO_PDU_H
#define PARSEO_PDU_H

#include <stddef.h>

/*
Límites del enunciado:
  - PDU total: hasta 122 bytes incluyendo delimitadores (\r\n).
  - timestamp: 1..11 chars decimales (0x30-0x39).
  - message:   1..100 chars ASCII en [0x20..0x7D] excluyendo 0x26 y 0x7C.   - end:       \r\n (0x0D 0x0A).
  - separador timestamp/message: '&' (0x26).
 */

#define PDU_MAX_LEN            122 // del enunciado 
#define PDU_TIMESTAMP_MAX      11 // 11 caracteres decimales permitidos
#define PDU_MESSAGE_MAX        100 //cantidad de caracteres del msj
#define PDU_PARSER_BUFFER_SIZE 8192  // definimos un tamaño de buffer interno para el parseo

#define PDU_DELTA_MAX_SEG      300

typedef struct {
    char   buffer[PDU_PARSER_BUFFER_SIZE];
    size_t len;   /* cuántos bytes útiles hay en buffer */
} pdu_parser_t;

void pdu_parser_init(pdu_parser_t *p);

int pdu_parser_feed(pdu_parser_t *p, const char *chunk, size_t n);


int pdu_parser_siguiente(pdu_parser_t *p,
                         char pdu_out[PDU_MAX_LEN + 1],
                         char parse_code_out[4],
                         int *enviar_out);

#endif /* PARSEO_PDU_H */