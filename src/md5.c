#include "md5.h"

#include <stdint.h> // para tipos de enteros de tamaño exacto, como uint32_t y uint64_t
#include <stdio.h> // snprintf
#include <string.h> // strlen, memcpy

/*
MD5 es una función criptográfica que toma datos como
entrada y devuelve un hash de 128 bits, convirtiendo
cualquier texto en 32 caracteres hex.
S: cuanto se rota en cada una de las 64 rondas
K: constantes del estandar (RFC 1321)
*/
static const uint32_t S[64] = {
    7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
    5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
    4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
    6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
};

static const uint32_t K[64] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a,
    0xa8304613, 0xfd469501, 0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821, 0xf61e2562, 0xc040b340,
    0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8,
    0x676f02d9, 0x8d2a4c8a, 0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70, 0x289b7ec6, 0xeaa127fa,
    0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92,
    0xffeff47d, 0x85845dd1, 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

/*
Rota x hacia la izquierda n bits
Los bits que salen por la izquierda vuelven a entrar por la derecha
*/
static uint32_t rotar_izq(uint32_t x, uint32_t n)
{
    return (x << n) | (x >> (32 - n));
}

/*
Lee 4 bytes como un numero de 32 bits en little-endian
*/
static uint32_t leer_palabra(const unsigned char *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

/*
Procesa un bloque de 64 bytes y actualiza el estado h[4]
*/
static void md5_bloque(uint32_t h[4], const unsigned char *bloque)
{
    // El bloque de 64 bytes se ve como 16 palabras de 4 bytes
    uint32_t M[16];
    for (int i = 0; i < 16; i++)
        M[i] = leer_palabra(bloque + 4 * i);

    uint32_t a = h[0], b = h[1], c = h[2], d = h[3];

    // 64 rondas en 4 tandas de 16
    // (cada tanda mezcla distinto y recorre las palabras en otro orden)
    for (int ronda = 0; ronda < 64; ronda++) {
        uint32_t mezcla, palabra;
        if (ronda < 16) {
            mezcla = (b & c) | (~b & d);
            palabra = ronda;
        } else if (ronda < 32) {
            mezcla = (d & b) | (~d & c);
            palabra = (5 * ronda + 1) % 16;
        } else if (ronda < 48) {
            mezcla = b ^ c ^ d;
            palabra = (3 * ronda + 5) % 16;
        } else {
            mezcla = c ^ (b | ~d);
            palabra = (7 * ronda) % 16;
        }
        uint32_t x = a + mezcla + K[ronda] + M[palabra];
        a = d;
        d = c;
        c = b;
        b = b + rotar_izq(x, S[ronda]);
    }

    h[0] += a;
    h[1] += b;
    h[2] += c;
    h[3] += d;
}

void md5(const char *texto, char salida[33])
{
    uint32_t h[4] = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476}; // estado inicial (definido por el estándar)
    const unsigned char *datos = (const unsigned char *)texto;
    size_t len = strlen(texto);

    // Bloques completos de 64 bytes
    size_t pos = 0;
    while (len - pos >= 64) {
        md5_bloque(h, datos + pos);
        pos += 64;
    }

    // Ultimo bloque con relleno: lo que sobra + 0x80 + ceros + largo en bits
    // Si no entra todo en 64 bytes, el relleno ocupa 2 bloques (128)
    size_t sobran = len - pos;
    unsigned char ultimo[128] = {0};
    memcpy(ultimo, datos + pos, sobran);
    ultimo[sobran] = 0x80;

    size_t tam_ultimo = (sobran + 1 + 8 <= 64) ? 64 : 128;
    uint64_t largo_bits = (uint64_t)len * 8;
    for (int k = 0; k < 8; k++) { // largo en los ultimos 8 bytes, little-endian
        ultimo[tam_ultimo - 8 + k] = (unsigned char)(largo_bits >> (8 * k));
    }

    md5_bloque(h, ultimo);
    if (tam_ultimo == 128) {
        md5_bloque(h, ultimo + 64);
    }

    // Resultado: los 4 numeros de h, byte por byte en little-endian, en hex
    char *p = salida;
    for (int k = 0; k < 4; k++) {
        for (int byte = 0; byte < 4; byte++) {
            p += snprintf(p, 3, "%02x", (h[k] >> (8 * byte)) & 0xff);
        }
    }
}
