
#include "../src/nose.h"
#include "../src/utils.h"

int main()
{
      char hash[65];
      gen_sha256("12345", strlen("12345"), hash);
      printf("%s\n", hash);
      return 0;
}