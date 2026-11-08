#ifndef CLIENTE_UDP_H
#define CLIENTE_UDP_H

#include <stddef.h>
#include <sys/types.h>

/*
 * Abre un socket UDP ya "conectado" al endpoint (servidor, puerto) y le
 * fija un timeout de recepción de `timeout_seg` segundos.
 *
 * Devuelve el file descriptor en caso de éxito, o -1 en error
 * (escribe el motivo por stderr con perror).
 */
int udp_abrir(const char *servidor, const char *puerto, int timeout_seg);

/*
 * Envía un datagrama por el socket ya abierto.
 * Devuelve bytes enviados, o -1 en error.
 */
ssize_t udp_enviar(int fd, const char *mensaje, size_t len);

/*
 * Espera un datagrama (hasta el timeout fijado en udp_abrir).
 * Si llega algo, lo escribe en `buffer`, le pone '\0' al final y devuelve
 * la cantidad de bytes útiles leídos (sin contar el '\0').
 *
 * Devuelve:
 *   > 0  bytes leídos
 *     0  no llegó nada dentro del timeout (o fin de datos)
 *   < 0  error de sistema
 */
ssize_t udp_recibir(int fd, char *buffer, size_t tam_buffer);

/*
 * Cierra el socket.
 */
void udp_cerrar(int fd);

#endif /* CLIENTE_UDP_H */