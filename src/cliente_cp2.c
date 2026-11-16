#include <stdio.h> // printf, fprintf, perror
#include <string.h> // funciones de texto y memoria
#include <unistd.h>

#include "cliente_tcp.h"
#include "cliente_udp.h"
#include "firma.h"
#include "ip_publica.h"
#include "parseo_pdu.h"

#define PUERTO_UDP_SERVER "20262"
#define TIMEOUT_SEG 10

int main(int argc, char *argv[])
{
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <servidor> <usuario> <secreto>\n", argv[0]);
        return 1;
    }
    const char *servidor = argv[1], *usuario = argv[2], *secreto = argv[3];

    // ---------- Checkpoint 1: autenticacion y puerto TCP ----------

    // Paso 1: averiguar la IP publica
    char ip[16];
    if (obtener_ip_publica(ip) != 0)
        return 1;
    printf("IP publica host: %s\n", ip);

    // Paso 2: armar usuario|ip|md5(usuario|ip|secreto)
    char mensaje[512];
    if (armar_pdu_auth(mensaje, sizeof mensaje, usuario, ip, secreto) == 0) {
        fprintf(stderr, "No se pudo armar la PDU de autenticación\n");
        return 1;
    }

    // Paso 3: abrir el socket UDP (lo usamos hasta el final)
    int fd_udp = udp_abrir(servidor, PUERTO_UDP_SERVER, TIMEOUT_SEG);
    if (fd_udp < 0)
        return 1;

    // Paso 4: mandar la auth y esperar una respuesta valida
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
        // Si no tiene código 200, es un error
        if (strncmp(resp, "200|", 4) != 0) {
            printf("El servidor rechazo el pedido: %s\n", resp);
            udp_cerrar(fd_udp);
            return 1;
        }

        // Separamos la firma (despues del ultimo '|') del resto
        char *ultimo = strrchr(resp, '|');
        if (!ultimo) continue;
        *ultimo = '\0';
        const char *firma_recibida = ultimo + 1;

        if (!validar_firma(resp, firma_recibida, secreto)) {
            printf("Firma invalida: se descarta y se sigue esperando.\n");
            continue;
        }

        // Tenemos 200|Port|17189, nos quedamos con el puerto
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
    printf("Autenticacion OK - Puerto efímero TCP: %s\n", puerto_tcp);

    // ---------- Checkpoint 2: conexion TCP, delimitacion y reenvio de PDUs ----------

    // Paso 5: conectarnos por TCP al puerto que nos dio el server
    int fd_tcp = tcp_abrir(servidor, puerto_tcp);
    if (fd_tcp < 0) {
        udp_cerrar(fd_udp);
        return 1;
    }

    // Paso 6: recibir las PDUs (delimitadas por \r\n) y reenviarlas por UDP
    // TCP no respeta los limites de las PDUs: una puede llegar partida en
    // varios recv o varias juntas, por eso las vamos acumulando en el parser
    pdu_parser_t parser = {0};
    char chunk[4096];
    int pdus_totales = 0, pdus_enviadas = 0;

    ssize_t rx;
    while ((rx = tcp_recibir(fd_tcp, chunk, sizeof chunk)) > 0) {
        if (pdu_parser_feed(&parser, chunk, (size_t)rx) < 0) break; // se llenó el buffer

        // Procesamos todas las PDUs que ya estén completas
        char pdu[PDU_PARSER_BUFFER_SIZE]; // entra cualquier PDU que entre en el parser
        char parse_code[4]; // 3 digitos + '\0'
        int enviar;
        int r; // 1 = saco una PDU, 0 = no hay PDU completa, -1 = error
        while ((r = pdu_parser_siguiente(&parser, pdu, parse_code, &enviar)) == 1) {
            pdus_totales++;
            printf("PDU #%d code=%s enviar=%d pdu=\"%s\"\n", pdus_totales, parse_code, enviar, pdu);

            if (!enviar) continue; // no se reenvia si tiene '|'

            // usuario|pdu|parse_code|md5(usuario|pdu|parse_code|secreto), por el mismo socket UDP
            char reenvio[PDU_PARSER_BUFFER_SIZE + 512]; // PDU + usuario, code y firma
            size_t m = armar_pdu_reenvio(reenvio, sizeof reenvio, usuario, pdu, parse_code, secreto);
            if (m == 0) {
                fprintf(stderr, "No se pudo armar PDU de reenvío\n");
                continue;
            }
            if (udp_enviar(fd_udp, reenvio, m) < 0) {
                perror("envio UDP (reenvío)");
                continue;
            }
            pdus_enviadas++;
        }
        if (r == -1) {
            fprintf(stderr, "Error del parser, abortando.\n");
            break;
        }
    }
    if (rx < 0) perror("recv TCP");

    // Paso 7: el server cerró la conexion -> cerramos todo y terminamos
    printf("Fin. PDUs delimitadas: %d, reenviadas por UDP: %d.\n", pdus_totales, pdus_enviadas);
    tcp_cerrar(fd_tcp);
    udp_cerrar(fd_udp);
    return 0;
}
