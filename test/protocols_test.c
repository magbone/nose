

#include "../src/protocols.h"
#include "../src/utils.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>


int main(void)
{
      struct EGRE_header *egre_header = filling_EGRE_header(0x0800, 0x1000);

      if (egre_header == NULL) return (-1);

      char *buffer = (char *)egre_header;

      for(int i = 0; i < sizeof(struct EGRE_header); i++)
      {
            printf("%02x ", buffer[i]);
      }

      printf("\n");

      struct PPP_LCP *lcp = (struct PPP_LCP *)malloc(sizeof(struct PPP_LCP) + 4);
      lcp->code = CONF_REQ;
      lcp->indentfier = 0x40;
      lcp->length = htons(sizeof(struct PPP_LCP) + 4);
      lcp->data = (char *)malloc(sizeof(4));
      memcpy(lcp->data, "1234", 4);
      
      buffer = (char *)lcp;
      for (int i = 0; i < sizeof(struct PPP_LCP) + 4; i++)
      {
            printf("%02x ", buffer[i]);
      }
      printf("\n");
      
      
      free(egre_header);
      return 0;
}