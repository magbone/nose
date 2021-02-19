

#include "crypt.h"
#include "protocols.h"

#include <mbedtls/config.h>
#include <mbedtls/aes.h>


int app_data_pkt(int id, char *payload, size_t payload_len, char *buffer)
{
      if (payload == NULL || payload_len <= 0)return (ERROR);

      struct vpn_proto_header* header = (struct vpn_proto_header *)malloc(sizeof(struct vpn_proto_header));

      if (header == NULL)
      {
            fprintf(stderr, "[ERROR] Malloc error\n");
            return (ERROR);
      }
      header->code = 0;
      header->type = APP_DATA;
      header->id = htons(id);
      header->length = htons(sizeof(struct vpn_proto_header) + payload_len);
      header->reversed = 0;

      memcpy(buffer, (char*)header, sizeof(struct vpn_proto_header));
      memcpy(buffer + sizeof(struct vpn_proto_header), payload, payload_len);

      free(header);
      return (sizeof(struct vpn_proto_header) + payload_len);     
}


int 
encrypt_by_aes_256(const char *plain_text, const int plain_text_len, char *cipher_text, char *key)
{
      unsigned char _iv[16] = {14, 31, 6, 126, 18, 12, 36, 70, 100, 9, 42, 51, 111, 84, 3, 25 };
      mbedtls_aes_context ctx;
      mbedtls_aes_setkey_enc(&ctx, (unsigned char *)key, 256);

      const int output_len = plain_text_len % 16 == 0 ? plain_text_len : (plain_text_len / 16 + 1) * 16;
      unsigned char input[output_len];
      memset(input, 0, plain_text_len);
      memcpy(input, plain_text, plain_text_len);

      if (0 != mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_ENCRYPT, output_len, (unsigned char *)_iv, input, (unsigned char *)cipher_text))
      {
            printf("[ERROR] Encrypt error\n");
            return (ERROR);
      }
      return (output_len);
}

int 
decrypt_by_aes_256(const char *cipher_text, const int cipher_text_len, char *plain_text, char *key)
{
      unsigned char _iv[16] = {14, 31, 6, 126, 18, 12, 36, 70, 100, 9, 42, 51, 111, 84, 3, 25 };
      mbedtls_aes_context ctx;
      mbedtls_aes_setkey_dec(&ctx, (unsigned char *)key, 256);

      if (cipher_text_len % 16 != 0) return (ERROR);
      unsigned char output[cipher_text_len];
      memset(output, 0, cipher_text_len);

      if (0 != mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_DECRYPT, cipher_text_len, (unsigned char *)_iv, (unsigned char *)cipher_text, (unsigned char *)plain_text))
      {
            printf("[ERROR] Decrypt error\n");
            return (OK);
      }
      return (cipher_text_len);
}