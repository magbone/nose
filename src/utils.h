

#ifndef _UTILS_H_
#define _UTILS_H_

#include "nose.h"

#define SHA256_LEN 32

int is_valid_ip_address(char *ip);

void swap(void *a, void *b, size_t size);

void gen_sha256(char *input, size_t len, char *output);

int combine_ser_chall_id(char *s_key, int challenge, int id, char* output);

void gen_random_int(int *value, int length);

void gen_random_node_id(char *node_id);

void gen_random_trans_id(char *trans_id);

int gethostbyname1(char *domain, char *ipv4);

#endif // !_UTILS_H_

