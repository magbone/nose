
#include "../src/crypt.h"

static inline void show_buf_as_hex(char *buf, int size)
{
      for (int i = 0; i < size; i++)
      {
            printf("%02x", buf[i] & 0xff);

            if ((i + 1) % 8 == 0) printf("\n");
            else printf(" ");
      }

      if (size % 8 != 0) printf("\n");
}

int main()
{
      unsigned char key[32] = "b2c3d4f5j23Asds9b2c3d4f5j23Asds9";

      unsigned char input[96] = {
            0x11, 0x4c,
  0x3a, 0x74, 0x5e, 0xf2, 0xbe, 0x90, 0xec, 0xa8,
  0x0f, 0x92, 0x25, 0xc0, 0x19, 0xd4, 0x12, 0xe7,
  0x1b, 0x29, 0x33, 0x65, 0x0a, 0x34, 0xc5, 0x1a,
  0xa3, 0x03, 0xfc, 0xcf, 0x06, 0xee, 0x01, 0xb6,
  0x1d, 0xf5, 0x2c, 0x02, 0x5d, 0x54, 0xd1, 0x5a,
  0xa2, 0x95, 0x06, 0x9e, 0x33, 0x24, 0xc9, 0xbe,
  0x82, 0x40, 0x02, 0xec, 0xc1, 0x87, 0x73, 0xbc,
  0xad, 0xc2, 0xff, 0xf7, 0x99, 0xd7, 0x5c, 0x6c,
  0x4e, 0xb3, 0x4f, 0x9e, 0x09, 0x1f, 0x78, 0x56,
  0xe5, 0x56, 0x9e, 0x51, 0xa6, 0xcc, 0xb2, 0x47,
  0x3f, 0x94, 0x9e, 0x18, 0x46, 0xd8, 0xce, 0x55,
  0x27, 0x48, 0x1f, 0xc6, 0xf4, 0xfa
      };

      unsigned char output[96] = {0};
      show_buf_as_hex((char *)input, 96);
      decrypt_by_aes_256((const char *)input, 112, (char *)output, (char *)key);

      
      printf("Plain text: \n");
      show_buf_as_hex((char *)input, 96);
      printf("Cipher text: \n");
      show_buf_as_hex((char *)output, 96);;

      unsigned char plain_text[96] = {0};
      encrypt_by_aes_256((const char *)output, 96, (char *)plain_text, (char *)key);
      printf("Plain text: \n");

      show_buf_as_hex((char *)plain_text, 96);
      return (0);
}