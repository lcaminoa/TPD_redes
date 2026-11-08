#include "md5.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>


static const uint32_t S[64] = {
    7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
    5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
    4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
    6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
};

static const uint32_t K[64] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

/* Rotación circular a la izquierda: los bits que salen por la izquierda
   vuelven a entrar por la derecha. Base de la "mezcla" de MD5. */
static uint32_t rotar_izq(uint32_t x, uint32_t n)
{
    return (x << n) | (x >> (32 - n));
}

/* Lee 4 bytes a partir de `p` y los interpreta como un uint32_t en
   little-endian, como pide el estándar MD5. */
static uint32_t leer_palabra(const unsigned char *p)
{
    return  (uint32_t)p[0]
         | ((uint32_t)p[1] <<  8)
         | ((uint32_t)p[2] << 16)
         | ((uint32_t)p[3] << 24);
}

/* Procesa un bloque de 64 bytes y actualiza el estado `h[4]`. 
static void md5_bloque(uint32_t h[4], const unsigned char *bloque)*/
{
    uint32_t M[16];
    for (int i = 0; i < 16; i++) M[i] = leer_palabra(bloque + 4 * i);

    uint32_t a = h[0], b = h[1], c = h[2], d = h[3];

    for (int i = 0; i < 64; i++) {
        uint32_t mezcla, palabra;
        if      (i < 16) { mezcla = (b & c) | (~b & d);  palabra = i; }
        else if (i < 32) { mezcla = (d & b) | (~d & c);  palabra = (5 * i + 1) % 16; }
        else if (i < 48) { mezcla = b ^ c ^ d;           palabra = (3 * i + 5) % 16; }
        else             { mezcla = c ^ (b | ~d);        palabra = (7 * i)     % 16; }

        uint32_t temp = d;
        d = c;
        c = b;
        b = b + rotar_izq(a + mezcla + K[i] + M[palabra], S[i]);
        a = temp;
    }

    h[0] += a; h[1] += b; h[2] += c; h[3] += d;
}

void md5(const char *texto, char salida[33])
{
   // Estado inicial del estándar 
    uint32_t h[4] = { 0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476 };

    const unsigned char *datos = (const unsigned char *)texto;
    size_t len = strlen(texto);
    size_t pos = 0;

   // 1. Procesar todos los bloques completos de 64 bytes 
    while (len - pos >= 64) {
        md5_bloque(h, datos + pos);
        pos += 64;
    }

   // 2. Armar el último bloque con padding:
          lo que sobró + 0x80 + ceros + longitud en bits (8 bytes) 
    size_t sobran = len - pos;
    unsigned char ultimo[128] = {0};  // 128 por si necesitamos 2 bloques 
    memcpy(ultimo, datos + pos, sobran);
    ultimo[sobran] = 0x80;

    size_t tam_ultimo = (sobran + 1 + 8 <= 64) ? 64 : 128;
    uint64_t largo_bits = (uint64_t)len * 8;
    for (int k = 0; k < 8; k++) {
        ultimo[tam_ultimo - 8 + k] = (unsigned char)(largo_bits >> (8 * k));
    }

    md5_bloque(h, ultimo);
    if (tam_ultimo == 128) md5_bloque(h, ultimo + 64);

   // 3. Volcar h[0..3] a hex en minúscula, little-endian por palabra 
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            unsigned char byte = (unsigned char)(h[i] >> (8 * j));
            snprintf(salida + (i * 8) + (j * 2), 3, "%02x", byte);
        }
    }
    salida[32] = '\0';
}