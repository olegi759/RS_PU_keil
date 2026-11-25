#include "ws_crypto.h"

// --- Алгоритм SHA-1 (исправленный) ---
#define ROL32(v, b) (((v) << (b)) | ((v) >> (32 - (b))))

void sha1_hash(const uint8_t *data, uint32_t len, uint8_t *hash_out) {
    uint32_t h0 = 0x67452301, h1 = 0xEFCDAB89, h2 = 0x98BADCFE, h3 = 0x10325476, h4 = 0xC3D2E1F0;
    uint32_t i = 0;

    // Обработка полных блоков по 64 байта
    for (; i + 63 < len; i += 64) {
        uint32_t w[80];
        for (int j = 0; j < 16; j++) {
            w[j] = (data[i+j*4] << 24) | (data[i+j*4+1] << 16) | (data[i+j*4+2] << 8) | data[i+j*4+3];
        }
        for (int j = 16; j < 80; j++) {
            w[j] = ROL32(w[j-3] ^ w[j-8] ^ w[j-14] ^ w[j-16], 1);
        }
        uint32_t a = h0, b = h1, c = h2, d = h3, e = h4;
        for (int j = 0; j < 80; j++) {
            uint32_t f, k;
            if (j < 20)      { f = (b & c) | ((~b) & d); k = 0x5A827999; }
            else if (j < 40) { f = b ^ c ^ d;            k = 0x6ED9EBA1; }
            else if (j < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDC; }
            else             { f = b ^ c ^ d;            k = 0xCA62C1D6; }
            uint32_t temp = ROL32(a, 5) + f + e + k + w[j];
            e = d; d = c; c = ROL32(b, 30); b = a; a = temp;
        }
        h0 += a; h1 += b; h2 += c; h3 += d; h4 += e;
    }

    // Padding (выравнивание)
    uint8_t pad[128] = {0};
    uint32_t rem = len - i;
    for (uint32_t j = 0; j < rem; j++) {
        pad[j] = data[i + j];
    }
    pad[rem] = 0x80;
    
    uint32_t pad_len = (rem < 56) ? 64 : 128;
    uint64_t bit_len = (uint64_t)len * 8;
    for (int j = 0; j < 8; j++) {
        pad[pad_len - 1 - j] = (bit_len >> (j * 8)) & 0xFF;
    }

    // Обработка блоков Padding
    for (uint32_t j = 0; j < pad_len; j += 64) {
        uint32_t w[80];
        for (int k = 0; k < 16; k++) {
            w[k] = (pad[j+k*4] << 24) | (pad[j+k*4+1] << 16) | (pad[j+k*4+2] << 8) | pad[j+k*4+3];
        }
        for (int k = 16; k < 80; k++) {
            w[k] = ROL32(w[k-3] ^ w[k-8] ^ w[k-14] ^ w[k-16], 1);
        }
        uint32_t a = h0, b = h1, c = h2, d = h3, e = h4;
        for (int k = 0; k < 80; k++) {
            uint32_t f, kk;
            if (k < 20)      { f = (b & c) | ((~b) & d); kk = 0x5A827999; }
            else if (k < 40) { f = b ^ c ^ d;            kk = 0x6ED9EBA1; }
            else if (k < 60) { f = (b & c) | (b & d) | (c & d); kk = 0x8F1BBCDC; }
            else             { f = b ^ c ^ d;            kk = 0xCA62C1D6; }
            uint32_t temp = ROL32(a, 5) + f + e + kk + w[k];
            e = d; d = c; c = ROL32(b, 30); b = a; a = temp;
        }
        h0 += a; h1 += b; h2 += c; h3 += d; h4 += e;
    }

    // Вывод результата
    hash_out[0] = (h0 >> 24) & 0xFF; hash_out[1] = (h0 >> 16) & 0xFF; hash_out[2] = (h0 >> 8) & 0xFF; hash_out[3] = h0 & 0xFF;
    hash_out[4] = (h1 >> 24) & 0xFF; hash_out[5] = (h1 >> 16) & 0xFF; hash_out[6] = (h1 >> 8) & 0xFF; hash_out[7] = h1 & 0xFF;
    hash_out[8] = (h2 >> 24) & 0xFF; hash_out[9] = (h2 >> 16) & 0xFF; hash_out[10] = (h2 >> 8) & 0xFF; hash_out[11] = h2 & 0xFF;
    hash_out[12] = (h3 >> 24) & 0xFF; hash_out[13] = (h3 >> 16) & 0xFF; hash_out[14] = (h3 >> 8) & 0xFF; hash_out[15] = h3 & 0xFF;
    hash_out[16] = (h4 >> 24) & 0xFF; hash_out[17] = (h4 >> 16) & 0xFF; hash_out[18] = (h4 >> 8) & 0xFF; hash_out[19] = h4 & 0xFF;
}

// --- Алгоритм Base64 (исправленный) ---
void base64_encode(const uint8_t *src, uint32_t len, char *dst) {
    static const char tbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    uint32_t i, j = 0;
    for (i = 0; i < len; i += 3) {
        uint32_t a = src[i];
        uint32_t b = (i + 1 < len) ? src[i + 1] : 0;
        uint32_t c = (i + 2 < len) ? src[i + 2] : 0;
        uint32_t triple = (a << 16) | (b << 8) | c;
        dst[j++] = tbl[(triple >> 18) & 0x3F];
        dst[j++] = tbl[(triple >> 12) & 0x3F];
        dst[j++] = (i + 1 < len) ? tbl[(triple >> 6) & 0x3F] : '=';
        dst[j++] = (i + 2 < len) ? tbl[triple & 0x3F] : '=';
    }
    dst[j] = '\0';
}