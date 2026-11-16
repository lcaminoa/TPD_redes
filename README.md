# TP D – Desarrollo de aplicaciones distribuidas con BSD sockets

Cliente que se autentica ante el servidor de la cátedra por UDP, obtiene un puerto TCP, recibe PDUs por TCP y las reenvía firmadas por UDP.

## Compilar

Hay dos programas: `cliente_cp1` (solo el checkpoint 1) y `cliente_cp2` (los dos desafíos, es la entrega final).

```
gcc -Wall -Wextra -Iinclude src/md5.c src/firma.c src/ip_publica.c src/cliente_udp.c src/cliente_cp1.c -o cliente_cp1
gcc -Wall -Wextra -Iinclude src/md5.c src/firma.c src/ip_publica.c src/cliente_udp.c src/cliente_tcp.c src/parseo_pdu.c src/cliente_cp2.c -o cliente_cp2
```
Compila sin warnings. En macOS `gcc` es `clang`, funciona igual.

## Ejecutar
```
./cliente_cp2 <servidor> <usuario> <secreto>
```
En nuestro caso:
```
./cliente_cp2 167.114.129.206 grupo16 <secreto>
```

Si sale bien, se ve algo así:
```
IP publica host: 190.1.2.3
Autenticacion OK - Puerto efímero TCP: 17189
PDU #1 code=200 enviar=1 pdu="1792303544&Temperatura ambiente 11 grados"
PDU #2 code=406 enviar=1 pdu="1762303544&Temperatura ambiente 11 grados"
...
Fin. PDUs delimitadas: 8, reenviadas por UDP: 7.
```

## Qué hace

### Checkpoint 1 (autenticacion)
1. Averigua la IP pública con un pedido HTTP escrito a mano sobre un socket TCP a `api.ipify.org` ([ip_publica.c](src/ip_publica.c)).
2. Arma `usuario|ip|firma`, con `firma = md5(usuario|ip|secreto)` ([firma.c](src/firma.c)). El MD5 está implementado a mano en [md5.c](src/md5.c).
3. Lo manda por UDP al puerto 20262 del server ([cliente_udp.c](src/cliente_udp.c)).
4. Espera la respuesta hasta 10 segundos:
   - `200` con firma válida -> se queda con el puerto TCP asignado
   - `200` con firma inválida -> la descarta y sigue esperando
   - `400` / `401` -> muestra que el servidor rechazó el pedido

### Checkpoint 2 (framing de PDUs)
5. Se conecta por TCP al puerto que dio el server ([cliente_tcp.c](src/cliente_tcp.c)).
6. Lee del socket y va acumulando los bytes en un buffer ([parseo_pdu.c](src/parseo_pdu.c)). Cada vez que aparece un `\r\n` saca una PDU, así funciona si una PDU llega partida en varios segmentos, si llegan varias juntas, o las dos cosas.
7. A cada PDU le asigna su parse code (200, 400, 405 a 410) y, si no tiene `|`, la reenvía por el mismo socket UDP como `usuario|pdu|parse_code|md5(usuario|pdu|parse_code|secreto)`.
8. Cuando el server cierra la conexión TCP, cierra todo y termina.

## Capturas
En [capturas/](capturas/) está la captura de Wireshark del checkpoint 1.
