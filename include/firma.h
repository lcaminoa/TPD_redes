#ifndef FIRMA_H
#define FIRMA_H

#include <stddef.h>

/*
Arma la PDU de autenticacion (checkpoint 1):
usuario|ip|md5(usuario|ip|secreto)
Devuelve cuantos bytes escribio en salida, o 0 si no entra
*/
size_t armar_pdu_auth(char *salida, size_t tam_salida, const char *usuario, const char *ip, const char *secreto);

/*
Arma la PDU de reenvio (checkpoint 2):
usuario|pdu|parse_code|md5(usuario|pdu|parse_code|secreto)
Devuelve cuantos bytes escribio en salida, o 0 si no entra
*/
size_t armar_pdu_reenvio(char *salida, size_t tam_salida, const char *usuario, const char *pdu,
                         const char *parse_code, const char *secreto);

/*
Chequea que la firma que mando el server sea md5(cuerpo|secreto)
cuerpo es la respuesta sin la firma del final (ej: "200|Port|17189")
Devuelve 1 si es valida, 0 si no
*/
int validar_firma(const char *cuerpo, const char *firma_recibida, const char *secreto);

#endif
