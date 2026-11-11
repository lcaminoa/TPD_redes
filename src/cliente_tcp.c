#include "cliente_tcp.h"

#include <stdio.h>
#include <unistd.h>
#include <netdb.h> //resolución de direcciones de hosts
#include <sys/socket.h> // sockets

int tcp_abrir(const char *servidor, const char *puerto)
{
    struct addrinfo pistas = {0}, *res = NULL;
    pistas.ai_family   = AF_INET;   //direcciones IPv4
    pistas.ai_socktype = SOCK_STREAM;  //socket TCP

    // resolución del servidor y puerto 
    int gai = getaddrinfo(servidor, puerto, &pistas, &res);
    
    if (gai != 0) {
        // error de resolución
        fprintf(stderr, "getaddrinfo TCP (%s:%s): %s\n",servidor, puerto, gai_strerror(gai));
        return -1;
    }
    //generamos un socket TCP con la resolución. 
    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    
    if (fd < 0) { 
        //error de creación del socket
        perror("socket TCP");
        freeaddrinfo(res);
        return -1;
    }

    // cuando establecemos la conec
    if (connect(fd, //socket descriptor
                 res->ai_addr, // ptr a un struct con la direccion del servidor
                 res->ai_addrlen)// tam de la estructura de la direccion) 
                 < 0 //fallo la conexion -> -1
                ) {
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
    //fd: socket a recibir, 
    // buffer: puntero a la zona de memoria donde se guardarán los datos recibidos
    // tam_buffer: cantidad máxima de bytes a recibir
    //0 : no le ponemos flags
    return recv(fd, buffer, tam_buffer, 0);
    //recv devuelve n: cantidad de bytes recibidos, 
    // 0: conexión cerrada, -1: error
}

void tcp_cerrar(int fd)
{
    // si es un descriptor válido, cerramos el socket TCP
    if (fd >= 0) close(fd);
}