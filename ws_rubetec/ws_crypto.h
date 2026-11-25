#ifndef WS_CRYPTO_H
#define WS_CRYPTO_H

#include <stdint.h>
#include <string.h>

void sha1_hash(const uint8_t *data, uint32_t len, uint8_t *hash_out);
void base64_encode(const uint8_t *src, uint32_t len, char *dst);

#endif