#ifndef CLIENTE_TCP_H
#define CLIENTE_TCP_H

#include <stddef.h>
#include <sys/types.h>

// Abre un socket TCP y se conecta a servidor:puerto. Devuelve el fd, o -1 si falló
int tcp_abrir(const char *servidor, const char *puerto);

/*
Lee hasta tam_buffer bytes. TCP es un chorro de bytes, asi que puede
devolver menos aunque falten datos por llegar
Devuelve los bytes leidos, 0 si el server cerró la conexion, o -1 si hubo error
*/
ssize_t tcp_recibir(int fd, char *buffer, size_t tam_buffer);

void tcp_cerrar(int fd);

#endif
