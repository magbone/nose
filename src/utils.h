

#ifndef _UTILS_H_
#define _UTILS_H_

#include "nose.h"

#define ODD_EVEN(arr, i, len) ( (i == len) ? 0x00 : arr[i]) // odd octets

int is_valid_ip_address(char *ip);

void swap(void *a, void *b);

u_int16_t checksum_cmpt(char * array, int len);

#endif // !_UTILS_H_

