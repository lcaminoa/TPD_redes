# TP D – Checkpoint #1

Cliente que se autentica ante el servidor de la cátedra por UDP y obtiene un puerto TCP.

## Compilar

```
gcc -Wall -Wextra -Iinclude src/md5.c src/firma.c src/cliente_udp.c src/cliente_tcp.c   src/ip_publica.c src/cliente_cpX.c  -o cliente_cpX
```
donde X cambia por 1 (checkpoint) o 2 general. 

Después cuando borremos checkpoint o lo dejemos comentado podemos compilar con: 

```
gcc -Wall -Wextra -Iinclude src/*.c -o cliente_cp2
```

(En Linux sería `gcc` en lugar de `clang`.)

## Ejecutar
```
./cliente <servidor> <usuario> <secreto>
```
En nuestro caso:
```
./cliente 167.114.129.206 grupo16 <secreto>
```

Si sale bien, se ve algo así:
```
IP publica: 190.1.2.3
Enviando:   grupo16|190.1.2.3|<firma>
Recibido:   200|Port|17189|<firma>
Autenticacion OK. Puerto TCP: 17189
```

## Qué hace
1. Averigua la IP pública con un pedido HTTP escrito a mano sobre un socket TCP a `api.ipify.org`.
2. Arma `usuario|ip|firma`, con `firma = md5(usuario|ip|secreto)`. El MD5 está implementado en el mismo archivo.
3. Lo manda por UDP al puerto 20262 del server.
4. Espera la respuesta hasta 10 segundos:
   - `200` con firma válida -> muestra el puerto TCP asignado
   - `200` con firma inválida -> la descarta y sigue esperando
   - `400` / `401` -> muestra que el servidor rechazó el pedido