#include "ip_publica.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int obtener_ip_publica(char ip[16])
{
    struct addrinfo pistas = {0}, *res;
    pistas.ai_family = AF_INET;
    pistas.ai_socktype = SOCK_STREAM;

    if (getaddrinfo("api.ipify.org", "80", &pistas, &res) != 0) {
        fprintf(stderr, "Error en getaddrinfo (api.ipify.org)\n");
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

    const char *pedido = "GET / HTTP/1.1\r\n"
                         "Host: api.ipify.org\r\n"
                         "Connection: close\r\n"
                         "\r\n";
    send(fd, pedido, strlen(pedido), 0);

    char buf[4096];
    size_t total = 0;
    ssize_t n;
    while ((n = recv(fd, buf + total, sizeof buf - 1 - total, 0)) > 0) {
        total += n;
    }
    close(fd);
    buf[total] = '\0';

    char *cuerpo = strstr(buf, "\r\n\r\n");
    if (cuerpo == NULL || sscanf(cuerpo + 4, "%15[0-9.]", ip) != 1) {
        fprintf(stderr, "Respuesta HTTP inesperada:\n%s\n", buf);
        return -1;
    }
    return 0;
}