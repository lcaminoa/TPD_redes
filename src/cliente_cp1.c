/*  #include <stdio.h>
#include <string.h>

#include "ip_publica.h"
#include "firma.h"
#include "cliente_udp.h"
//SACAR ESTE IMPORT DE TCP 
#include "cliente_tcp.h"

#define PUERTO_UDP_SERVER   "20262"
#define TIMEOUT_SEG         10

int main(int argc, char *argv[])
{
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <servidor> <usuario> <secreto>\n", argv[0]);
        return 1;
    }
    const char *servidor = argv[1];
    const char *usuario  = argv[2];
    const char *secreto  = argv[3];

    // 1. Averiguar la IP pública mediante un servicio de web
    char ip[16];
    if (obtener_ip_publica(ip) != 0) return 1;
    printf("IP publica: %s\n", ip);

    //2. Armar usuario|ip|md5(usuario|ip|secreto)
    char mensaje[512];
    if (armar_pdu_auth(mensaje, sizeof mensaje,
                       usuario, ip, secreto) == 0) {
        fprintf(stderr, "No se pudo armar la PDU de autenticación\n");
        return 1;
    }

    // 3. Abrir socket UDP con timeout 10segs
    int fd = udp_abrir(servidor, PUERTO_UDP_SERVER, TIMEOUT_SEG);
    if (fd < 0) return 1;

    // 4. Mandar y leer en loop, descartando respuestas con firma inválida 
    if (udp_enviar(fd, mensaje, strlen(mensaje)) < 0) {
        perror("send UDP");
        udp_cerrar(fd);
        return 1;
    }

    char resp[512];
    ssize_t n;
    while ((n = udp_recibir(fd, resp, sizeof resp)) > 0) {

        // Tiene que empezar con "200|"; si no, es 400/401 
        if (strncmp(resp, "200|", 4) != 0) {
            printf("El servidor rechazo el pedido: %s\n", resp);
            udp_cerrar(fd);
            return 1;
        }

        // Separar "cuerpo | firma" partiendo por el último '|' 
        char *ultimo = strrchr(resp, '|');
        if (!ultimo) continue;
        *ultimo = '\0';
        const char *firma_recibida = ultimo + 1;

        // Validar firma: si no cierra, descartar y seguir esperando 
        if (!validar_firma(resp, firma_recibida, secreto)) {
            printf("Firma invalida: se descarta y se sigue esperando.\n");
            continue;
        }

        // Tenemos una rta 200|Port|17189 nos quedamos con el port 
        printf("Autenticacion OK. Puerto TCP: %s\n",
               resp + strlen("200|Port|"));
        udp_cerrar(fd);
        return 0;
    }
    // cerramos la conexion UDP 
    printf("No llego respuesta valida en %d segundos.\n", TIMEOUT_SEG);
    udp_cerrar(fd);
    return 1;
} 
 */