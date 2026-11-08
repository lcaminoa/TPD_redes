#ifndef IP_PUBLICA_H
#define IP_PUBLICA_H

/*
 * Consulta api.ipify.org por HTTP/1.1 (sin libcurl, a mano, como pide el TP)
 * y escribe la IP pública en `ip` (hasta 15 chars + '\0'; buffer de 16).
 *
 * Devuelve 0 en éxito, -1 en error.
 */
int obtener_ip_publica(char ip[16]);

#endif /* IP_PUBLICA_H */