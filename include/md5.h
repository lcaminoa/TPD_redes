#ifndef MD5_H
#define MD5_H

/*
 * Calcula el digesto MD5 del string `texto` y lo escribe en `salida`
 * como 32 chars hexadecimales en minúscula + '\0' (33 bytes totales).
 */
void md5(const char *texto, char salida[33]);

#endif /* MD5_H */