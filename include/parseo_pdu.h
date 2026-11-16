#ifndef PARSEO_PDU_H
#define PARSEO_PDU_H

#include <stddef.h>
#include <stdio.h>

/*
Limites del enunciado:
PDU = <timestamp>&<message>\r\n, hasta 122 bytes contando el \r\n
timestamp: 1 a 11 digitos (0x30-0x39)
message: 1 a 100 chars entre 0x20 y 0x7D, sin '&' (0x26) ni '|' (0x7C)
Una PDU mas larga solo puede ser por timestamp o message muy largos (407 o 410)
*/
#define PDU_TIMESTAMP_MAX 11 // 11 caracteres decimales permitidos
#define PDU_MESSAGE_MAX 100 // cantidad de caracteres del msj
#define PDU_PARSER_BUFFER_SIZE 8192 // tamaño del buffer interno para el parseo

#define PDU_DELTA_MAX_SEG 300 // diferencia maxima con la hora actual

typedef struct {
    char buffer[PDU_PARSER_BUFFER_SIZE];
    size_t len; // cuantos bytes utiles hay en buffer
} pdu_parser_t;

// Agrega al buffer del parser los n bytes que llegaron por TCP. Devuelve -1 si no entran
int pdu_parser_feed(pdu_parser_t *p, const char *chunk, size_t n);

/*
Saca la siguiente PDU completa (hasta el \r\n) del buffer, sin el \r\n
Le asigna el parse_code y en enviar_out dice si hay que reenviarla
Devuelve 1 si saco una PDU, 0 si todavia no hay ninguna completa
*/
int pdu_parser_siguiente(pdu_parser_t *p, char pdu_out[PDU_PARSER_BUFFER_SIZE], char parse_code_out[4], int *enviar_out);

#endif
