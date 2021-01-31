

#ifndef _UTILS_H_
#define _UTILS_H_

#include "nose.h"


int is_valid_ip_address(char *ip);

void swap(void *a, void *b, size_t size);

u_int16_t checksum_cmpt(char * array, int len);

#endif // !_UTILS_H_

