
#include "../src/key_neg.h"

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
      struct key_neg a, b;

      
      if ( OK != key_neg_make_public( &a ) || OK != key_neg_make_public( &b ) )
            return ( 0 );
      printf( "A public key: \n" );
      show_buf_as_hex( (char *) a.pub_key, 32 );
      printf( "B public key: \n" );
      show_buf_as_hex( (char *) b.pub_key, 32 );
      if ( OK != key_neg_make_secret( &a, b.pub_key, 32 ) || OK != key_neg_make_secret( &b, a.pub_key, 32 )) 
            return ( 0 );     
      printf( "A secret key: \n" );
      show_buf_as_hex( (char *) a.sec_key, 32 );
      printf( "B secret key: \n" );
      show_buf_as_hex( (char *) b.sec_key, 32 );
      return ( 0 );
}