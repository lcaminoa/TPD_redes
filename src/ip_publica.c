#include "ip_publica.h"

#include <arpa/inet.h> // funciones para manejar direcciones IP
#include <netdb.h> // para getaddrinfo y freeaddrinfo
#include <stdio.h> // fprintf, sscanf, perror
#include <string.h> // strlen, strstr
#include <sys/socket.h> // para BSD sockets
#include <unistd.h> // close, para cerrar el socket

int obtener_ip_publica(char ip[16])
{
    struct addrinfo pistas = {0}, *res;
    pistas.ai_family = AF_INET; // IPv4
    pistas.ai_socktype = SOCK_STREAM; // TCP
    if (getaddrinfo("api.ipify.org", "80", &pistas, &res) != 0) { // DNS
        fprintf(stderr, "No se pudo resolver api.ipify.org\n");
        return -1;
    }
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0 || connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
        perror("connect api.ipify.org");
        if (fd >= 0) close(fd);
        freeaddrinfo(res);
        return -1;
    }
    freeaddrinfo(res);

    // Pedido HTTP (es texto y la linea vacia final marca el fin)
    const char *pedido = "GET / HTTP/1.1\r\n"
                         "Host: api.ipify.org\r\n"
                         "Connection: close\r\n"
                         "\r\n";
    send(fd, pedido, strlen(pedido), 0);

    // TCP es un chorro de bytes: leemos en un while hasta que cierre
    char buf[4096];
    size_t total = 0;
    ssize_t n;
    while ((n = recv(fd, buf + total, sizeof buf - 1 - total, 0)) > 0) {
        total += n;
    }
    close(fd);
    buf[total] = '\0';

    // La IP es el cuerpo (lo q viene despues de "\r\n\r\n")
    char *cuerpo = strstr(buf, "\r\n\r\n");
    if (cuerpo == NULL || sscanf(cuerpo + 4, "%15[0-9.]", ip) != 1) {
        fprintf(stderr, "Respuesta HTTP inesperada:\n%s\n", buf);
        return -1;
    }
    return 0;
}
