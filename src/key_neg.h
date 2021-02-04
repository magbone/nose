
#ifndef _KEY_NEG_H_
#define _KEY_NEG_H_

#include "nose.h"

// TODO
struct key_neg{
      u_int8_t *pub_key;      
};

int key_neg_pkt(int id, u_int8_t *pub_key, size_t key_len, char *buffer);

int key_neg_pkt_validate(char *buffer, u_int8_t *pub_key, size_t key_len);

#endif // !_KEY_NEG_H_