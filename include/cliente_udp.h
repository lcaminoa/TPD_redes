#ifndef CLIENTE_UDP_H
#define CLIENTE_UDP_H

#include <stddef.h>
#include <sys/types.h>

/*
Abre un socket UDP "conectado" a servidor:puerto, con un limite
de timeout_seg segundos para recibir
Devuelve el fd, o -1 si falló
*/
int udp_abrir(const char *servidor, const char *puerto, int timeout_seg);

// Manda un datagrama. Devuelve los bytes enviados, o -1 si falló
ssize_t udp_enviar(int fd, const char *mensaje, size_t len);

/*
Espera un datagrama y le agrega el '\0' al final
Devuelve los bytes leidos, 0 si se venció el timeout, o -1 si hubo error
*/
ssize_t udp_recibir(int fd, char *buffer, size_t tam_buffer);

void udp_cerrar(int fd);

#endif
