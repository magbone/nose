
#ifndef _CRYPT_H_
#define _CRYPT_H_

#include "nose.h"

int app_data_pkt(int id, char *payload, size_t payload_len, char *buffer);

int encrypt_by_aes_256(const char *plain_text, const int plain_text_len, char *cipher_text, char *key);

int decrypt_by_aes_256(const char *cipher_text, const int cipher_text_len, char *plain_text, char *key);

#endif // !_CRYPT_H_