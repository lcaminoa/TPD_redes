#ifndef FIRMA_H
#define FIRMA_H

#include <stddef.h>

/*
 * Arma la PDU de autenticación del checkpoint 1:
 *   <usuario>|<ip>|md5(<usuario>|<ip>|<secreto>)
 *
 * Devuelve la cantidad de bytes escritos en `salida` (sin contar el '\0'),
 * o 0 si no entra en `tam_salida`.
 */
size_t armar_pdu_auth(char *salida, size_t tam_salida,
                      const char *usuario,
                      const char *ip,
                      const char *secreto);

/*
 * Arma la PDU de reenvío del checkpoint 2:
 *   <usuario>|<pdu>|<parse_code>|md5(<usuario>|<pdu>|<parse_code>|<secreto>)
 *
 * Devuelve la cantidad de bytes escritos (sin '\0'), o 0 si no entra.
 */
size_t armar_pdu_reenvio(char *salida, size_t tam_salida,
                         const char *usuario,
                         const char *pdu,
                         const char *parse_code,
                         const char *secreto);

/*
 * Verifica que la firma hex que vino del server coincida con
 *   md5(<cuerpo>|<secreto>)
 *
 * `cuerpo` es el mensaje recibido sin su firma final (p.ej. "200|Port|17189").
 * `firma_recibida` es el string hex de 32 chars que vino después del
 * último '|' del mensaje original.
 *
 * Devuelve 1 si válida, 0 si no.
 */
int validar_firma(const char *cuerpo,
                  const char *firma_recibida,
                  const char *secreto);

#endif /* FIRMA_H */