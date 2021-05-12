




#include "utils.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>
#include <mbedtls/sha256.h>

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


void 
gen_sha256(char *input, size_t len, char *output)
{
      unsigned char hash[SHA256_LEN];
      mbedtls_sha256_context ctx;
      mbedtls_sha256_init( &ctx );
      mbedtls_sha256_starts( &ctx, 0);
      mbedtls_sha256_update( &ctx, (const unsigned char *)input, len );
      mbedtls_sha256_finish( &ctx, hash);

      for( int i = 0; i < SHA256_LEN; i++ )
            sprintf( output + ( i * 2 ), "%02x", hash[i] );
      
      output[64] = 0;

      mbedtls_sha256_free( &ctx );
}

/**
 *  Combine secret, challenge and id.
*/
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

void 
gen_random_int(int *value, int length)
{
      int max = (1 << length) - 1;
      srand((unsigned)time(NULL));
      *value = rand() % max;
}

void 
gen_random_node_id(char *node_id)
{

      const char base[] = {"0123456789abcdefghijklmnopqrstuvwxyz"};
      srand((unsigned)time(NULL));
      if (node_id == NULL)
            return;
      
      for (int i = 0; i < 20; i++)
            node_id[i] = base[rand() % strlen(base)];
      node_id[20] = '\0';
}

void 
gen_random_trans_id(char *trans_id)
{
      srand((unsigned)time(NULL));
      if (trans_id == NULL)
            return;
      
      for (int i = 0; i < 16; i++)
            trans_id[i] = rand() % 255;
}

int 
gethostbyname1(char *domain, char *ipv4)
{
      if (domain == NULL || ipv4 == NULL) return (ERROR);
      char *ptr, **pptr;
      struct hostent *hp = gethostbyname(domain);
      
      if (hp == NULL) return (ERROR);

      switch (hp->h_addrtype)
      {
      case AF_INET:
            pptr = hp->h_addr_list;

            for (; *pptr != NULL; pptr++)
            {
                  ptr = inet_ntoa( *(struct in_addr*)*pptr );
                  if (ptr == NULL) return (ERROR);
                  strncpy(ipv4, ptr, strlen(ptr));
                  break;
            }
            
            break;
      
      default:
            break;
      }

      return (OK);
}