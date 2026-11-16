#ifndef IP_PUBLICA_H
#define IP_PUBLICA_H

/*
Averigua la IP publica con un pedido HTTP a mano a api.ipify.org
Devuelve 0 si pudo, -1 si falló
*/
int obtener_ip_publica(char ip[16]);

#endif
