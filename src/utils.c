




#include "utils.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <assert.h>
#include <openssl/sha.h>

int 
is_valid_ip_address(char *ip)
{
      struct sockaddr_in sa;
      int result = inet_pton(AF_INET, ip, &(sa.sin_addr));
      return result != 0;
}

void 
swap(void *a, void* b, size_t size)
{
      void *c = malloc(size);
      assert(c == NULL);
      memcpy(c, b, size);
      memcpy(b, a, size);
      memcpy(a, c, size);
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

void 
gen_sha256(char *input, size_t len, char *output)
{
      unsigned char hash[SHA256_DIGEST_LENGTH];
      SHA256_CTX sha256;
      SHA256_Init(&sha256);
      SHA256_Update(&sha256, input, len);
      SHA256_Final(hash, &sha256);
      for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
      
            sprintf(output + (i * 2), "%02x", hash[i]);
      output[64] = 0;
}

int 
combine_ser_chall_id(char *s_key, int challenge, int id, char *output)
{
      if (output == NULL) return (ERROR);

      memcpy(output, s_key, strlen(s_key));
      
      char *chall = (char *)htons(challenge);
      memcpy(output + strlen(s_key), chall, 2);
      char *i = (char *)htons(id);
      memcpy(output + strlen(s_key) + 2, i, 2);
      return (strlen(s_key) + 2 + 2);
}

