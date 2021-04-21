
#include "../src/nose.h"
#include "../src/conf/conf-reader.h"


int main()
{
      struct conf_reader cread;
      if (read_conf(&cread, "peer_a.conf", "r") != OK)
      {
            printf("Read failed\n");
            return 0;
      }
      char c[MAX];
      if (get_value(&cread, "vlan_remote_ipv4p", c) == OK)
            printf("vlan_remote_ipv4: %s\n", c);
      else {
            printf("Not found\n");
      }
      return 0;
}