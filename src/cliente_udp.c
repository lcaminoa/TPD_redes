#include "cliente_udp.h"

#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

int udp_abrir(const char *servidor, const char *puerto, int timeout_seg)
{
    struct addrinfo pistas = {0}, *res = NULL;
    pistas.ai_family   = AF_INET;     // IPv4.
    pistas.ai_socktype = SOCK_DGRAM;   // UDP.
    // resolución del servidor y puerto 
    int gai = getaddrinfo(servidor, puerto, &pistas, &res);
    if (gai != 0) {
        fprintf(stderr, "getaddrinfo UDP (%s:%s): %s\n",
                servidor, puerto, gai_strerror(gai));
        return -1;
    }
// socket en funcion de la resolución obtenida 
    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    //socket devuelve -1 si hubo error, sino devuelve un descriptor de archivo
    if (fd < 0) {
        perror("socket UDP");
        freeaddrinfo(res);
        return -1;
    }


     /* En UDP, connect() fija el servidor de destino y permite usar
         send() y recv() sin indicar la dirección en cada operación. */
    if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
        perror("connect UDP");
        close(fd);
        freeaddrinfo(res);
        return -1;
    }
    freeaddrinfo(res);

    struct timeval limite = { .tv_sec = timeout_seg, .tv_usec = 0 };
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &limite, sizeof limite) < 0) {
        perror("setsockopt SO_RCVTIMEO");
        close(fd);
        return -1;
    }

    return fd;
}

ssize_t udp_enviar(int fd, const char *mensaje, size_t len)
{
    return send(fd, mensaje, len, 0);
}

ssize_t udp_recibir(int fd, char *buffer, size_t tam_buffer)
{
    if (tam_buffer == 0) return -1;

    // Dejamos un byte libre para agregar el terminador '\0'.
    ssize_t n = recv(fd, buffer, tam_buffer - 1, 0);
    if (n < 0) {
        // EAGAIN/EWOULDBLOCK indica que venció el tiempo de espera.
        if (errno == EAGAIN || errno == EWOULDBLOCK) return 0;
        return -1;
    }
    buffer[n] = '\0';
    return n;
}

void udp_cerrar(int fd)
{
    if (fd >= 0) close(fd);
}