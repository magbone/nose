




#include "utils.h"
#include <arpa/inet.h>
#include <ctype.h>

int 
is_valid_ip_address(char *ip)
{
      struct sockaddr_in sa;
      int result = inet_pton(AF_INET, ip, &(sa.sin_addr));
      return result != 0;
}

void 
swap(void *a, void* b)
{
      void *c = malloc(sizeof(b));
      memcpy(c, b, sizeof(b));
      memcpy(b, a, sizeof(a));
      memcpy(a, c, sizeof(c));
      free(c);
}

u_int16_t 
checksum_cmpt(char * array, int len)
{
      int temp_len = len % 2 != 0 ? len / 2 + 1 : len / 2;
      u_int16_t temp_check_sum[temp_len];
      u_int16_t checksum;
      memset(temp_check_sum, 0, temp_len);
      for(int i = 0, j = 0; i < temp_len; i++,j+=2)
            temp_check_sum[i] = (array[j] << 8) + (ODD_EVEN(array, j + 1, len) & 0xff);
      
      int k = 0, sum = 0;

      while(k < temp_len)
      {
            sum += temp_check_sum[k];
            k++;
      }
      
      while(sum >> 16 != 0) sum = (sum & 0xffff) + (sum >> 16);
      return (u_int16_t)~sum;
}
