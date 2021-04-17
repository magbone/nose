
#include "../src/stun/nat_test.h"

const char *nat_type_strs[] = {
      "UDP Blocked",
      "Sym. UDP Firewall",
      "Open Internet",
      "Full Cone",
      "Restricted",
      "Port Restricted",
      "Symmetric NAT",
      "Unexpected NAT Type",
      0
};

int main()
{
      char* source_addr = "10.66.83.203";
      struct nat_type type;
      if (get_nat_type(source_addr, 0, NULL, 0, &type))
      {
            printf("NAT Type: %s\n"
                  "External IP: %s\n"
                  "External Port: %d\n", 
                  nat_type_strs[type.nat_type],
                  type.ipv4,
                  type.port);
      }
      return 0;
}