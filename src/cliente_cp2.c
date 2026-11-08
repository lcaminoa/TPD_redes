#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ip_publica.h"
#include "firma.h"
#include "cliente_udp.h"
#include "cliente_tcp.h"
#include "parseo_pdu.h"

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

    // _________Autenticación y obtención del puerto TCP dinámico_______
    // 1. Averiguar la IP pública mediante un servicio de web
    char ip[16];
    if (obtener_ip_publica(ip) != 0) return 1;
    printf("IP publica: %s\n", ip);

    //2. Armar usuario|ip|md5(usuario|ip|secreto)
    char mensaje[512];
    if (armar_pdu_auth(mensaje, sizeof mensaje, usuario, ip, secreto) == 0) {
        fprintf(stderr, "No se pudo armar la PDU de autenticación\n");
        return 1;
    }

    // 3. Abrir socket UDP - hasta el final
    int fd_udp = udp_abrir(servidor, PUERTO_UDP_SERVER, TIMEOUT_SEG);
    if (fd_udp < 0) return 1;

    
    // 4. Enviar auth y esperar respuesta válida 
    if (udp_enviar(fd_udp, mensaje, strlen(mensaje)) < 0) {
        perror("send UDP (auth)");
        udp_cerrar(fd_udp);
        return 1;
    }

    char puerto_tcp[16] = {0};
    int autenticado = 0;
    char resp[512];
    ssize_t n;
    while ((n = udp_recibir(fd_udp, resp, sizeof resp)) > 0) {
        if (strncmp(resp, "200|", 4) != 0) {
            printf("El servidor rechazo el pedido: %s\n", resp);
            udp_cerrar(fd_udp);
            return 1;
        }
        char *ultimo = strrchr(resp, '|');
        if (!ultimo) continue;
        *ultimo = '\0';
        const char *firma_recibida = ultimo + 1;

        if (!validar_firma(resp, firma_recibida, secreto)) {
            printf("Firma invalida: se descarta y se sigue esperando.\n");
            continue;
        }

        const char *port_str = resp + strlen("200|Port|");
        size_t port_len = strlen(port_str);
        if (port_len >= sizeof puerto_tcp) port_len = sizeof puerto_tcp - 1;
        memcpy(puerto_tcp, port_str, port_len);
        puerto_tcp[port_len] = '\0';
        autenticado = 1;
        break;
    }
    if (!autenticado) {
        printf("No llego respuesta valida en %d segundos.\n", TIMEOUT_SEG);
        udp_cerrar(fd_udp);
        return 1;
    }
    printf("Autenticacion OK. Puerto TCP: %s\n", puerto_tcp);

    //_______________Conexión TCP, framing y reenvío de PDUs________________

    // 5. Abrir TCP al puerto dinámico que nos dio el server 
    int fd_tcp = tcp_abrir(servidor, puerto_tcp);
    if (fd_tcp < 0) {
        // al ocurrir un error cerramos la conexion UDP
        udp_cerrar(fd_udp);
        return 1;
    }

    // tenemos un descriptor de TCP en fd_tcp 
    pdu_parser_t parser;
    pdu_parser_init(&parser);

    char chunk[4096];
    int pdus_totales = 0, pdus_enviadas = 0;
    ssize_t rx;
    while ((rx = tcp_recibir(fd_tcp, chunk, sizeof chunk)) > 0) {

        if (pdu_parser_feed(&parser, chunk, (size_t)rx) < 0) {
            fprintf(stderr, "Buffer del parser lleno — abortando.\n");
            break;
        }

        // Drenar TODAS las PDUs delimitadas que haya en el buffer.
        //Un solo tcp_recibir puede traer varias PDUs ya completas. 
        char pdu[PDU_MAX_LEN + 1];
        char parse_code[4];
        int enviar;
        int r;
        while ((r = pdu_parser_siguiente(&parser, pdu, parse_code,
                                         &enviar)) == 1) {
            pdus_totales++;
            printf("PDU #%d code=%s enviar=%d pdu=\"%s\"\n",
                   pdus_totales, parse_code, enviar, pdu);

            if (!enviar) continue;

            // Armar PDU de reenvío y despacharla por el MISMO socket UDP */
            char reenvio[512];
            size_t m = armar_pdu_reenvio(reenvio, sizeof reenvio,
                                         usuario, pdu, parse_code, secreto);
            if (m == 0) {
                fprintf(stderr, "No se pudo armar PDU de reenvío\n");
                continue;
            }
            if (udp_enviar(fd_udp, reenvio, m) < 0) {
                perror("send UDP (reenvío)");
                continue;
            }
            pdus_enviadas++;
        }
        if (r == -1) {
            fprintf(stderr, "Error del parser — abortando.\n");
            break;
        }
    }
    if (rx < 0) perror("recv TCP");

    // 7. EOF del server → cerrar todo y terminar */
    printf("Fin. PDUs delimitadas: %d, reenviadas por UDP: %d.\n",
           pdus_totales, pdus_enviadas);

    tcp_cerrar(fd_tcp);
    udp_cerrar(fd_udp);
    return 0;
}