#ifndef CLIENTE_TCP_H
#define CLIENTE_TCP_H

#include <stddef.h>
#include <sys/types.h>
  
/*
 * Abre un socket TCP y hace connect() al endpoint (ip, puerto).
 * Devuelve el file descriptor en éxito, o -1 en error (perror + stderr).
 */
int tcp_abrir(const char *servidor, const char *puerto);
 
/*
 * Envía `len` bytes por el socket. Nota: en TCP, send() puede mandar
 * MENOS bytes de los pedidos (el buffer del kernel podría estar lleno).
 * Para mandar todo, el llamador tiene que hacer un loop con el offset.
 * Para este TP (donde el cliente apenas manda datos por TCP) la versión
 * simple alcanza.
 *
 * Devuelve bytes enviados, o -1 en error.
 */
ssize_t tcp_enviar(int fd, const char *buf, size_t len);
 
/*
 * Lee hasta `tam_buffer` bytes del stream. TCP puede entregar menos bytes
 * de los pedidos aunque haya más por venir — eso es normal y no es un
 * error.
 *
 * Devuelve:
 *   > 0  bytes leídos efectivamente
 *     0  EOF: el otro lado cerró la conexión (fin de stream)
 *   < 0  error de sistema
 */
ssize_t tcp_recibir(int fd, char *buffer, size_t tam_buffer);
 
/*
 * Cierra el socket (dispara el 4-way handshake de finalización).
 */
void tcp_cerrar(int fd);
 
#endif /* CLIENTE_TCP_H */
 