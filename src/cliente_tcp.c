#include "cliente_tcp.h"

#include <netdb.h> // para getaddrinfo y freeaddrinfo
#include <stdio.h> // fprintf, perror
#include <sys/socket.h> // para BSD sockets
#include <unistd.h> // close, para cerrar el socket

int tcp_abrir(const char *servidor, const char *puerto)
{
    struct addrinfo pistas = {0}, *res = NULL;
    pistas.ai_family = AF_INET; // IPv4
    pistas.ai_socktype = SOCK_STREAM; // TCP
    int gai = getaddrinfo(servidor, puerto, &pistas, &res);
    if (gai != 0) {
        fprintf(stderr, "getaddrinfo TCP (%s:%s): %s\n", servidor, puerto, gai_strerror(gai));
        return -1;
    }
    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd < 0) {
        perror("socket TCP");
        freeaddrinfo(res);
        return -1;
    }

    // Establecemos la conexion (3-way handshake)
    if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
        fprintf(stderr, "error al conectar TCP %s:%s\n", servidor, puerto);
        perror("Descripción del error:");
        close(fd);
        freeaddrinfo(res);
        return -1;
    }
    freeaddrinfo(res);
    return fd;
}

ssize_t tcp_recibir(int fd, char *buffer, size_t tam_buffer)
{
    // recv devuelve los bytes recibidos, 0 si se cerró la conexion, -1 si hubo error
    return recv(fd, buffer, tam_buffer, 0);
}

void tcp_cerrar(int fd)
{
    if (fd >= 0) close(fd);
}
