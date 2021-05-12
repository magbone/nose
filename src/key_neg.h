
#ifndef _KEY_NEG_H_
#define _KEY_NEG_H_

#include "nose.h"

#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/ecdh.h>

// ECDH

struct key_neg{
      mbedtls_ecdh_context ctx;
      mbedtls_ctr_drbg_context ctr_drbg;
      mbedtls_entropy_context entropy;
      uint8_t pub_key[32]; 
      uint8_t sec_key[32];     
};


int key_neg_make_public( struct key_neg *neg );
int key_neg_make_secret( struct key_neg *neg, uint8_t *public_key, size_t pklen );

int key_neg_pkt(int id, u_int8_t *pub_key, size_t key_len, char *buffer);

int key_neg_pkt_validate(char *buffer, u_int8_t *pub_key, size_t key_len);

#endif // !_KEY_NEG_H_