

#ifndef _UTILS_H_
#define _UTILS_H_

#include "nose.h"

#define ODD_EVEN(arr, i, len) ( (i == len) ? 0x00 : arr[i]) // odd octets

int is_valid_ip_address(char *ip);

void swap(void *a, void *b, size_t size);

u_int16_t checksum_cmpt(char * array, int len);

void gen_sha256(char *input, size_t len, char *output);

int combine_ser_chall_id(char *s_key, int challenge, int id, char* output);



#endif // !_UTILS_H_

