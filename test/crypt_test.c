
#include "../src/crypt.h"


int main()
{
      unsigned char key[16] = {0x12, 0x43, 0x34, 0x88, 
      0xad, 0x67, 0x78, 0xa0, 
      0x12, 0xed, 0xac, 0x59,
      0x91, 0x88, 0x43, 0x67};

      unsigned char input[16] = "213";

      unsigned char output[16] = {0};

      encrypt_by_aes_256((const char *)input, 16, (char *)output, (char *)key);

      printf("Plain text: %s\n", input);
      printf("Cipher text:");
      for(int i = 0; i < 16; i++)
            printf("%02x", output[i]);
      printf("\n");

      unsigned char plain_text[16] = {0};
      decrypt_by_aes_256((const char *)output, 16, (char *)plain_text, (char *)key);
      printf("Plain text: %s\n", plain_text);
      return (0);
}