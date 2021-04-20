
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

int main(int argc, char **argv)
{
      if (argc < 3) return 0;
      char* source_addr = argv[1];
      char* stun_sever_addr = argv[2];
      struct nat_type type;
      if (get_nat_type(source_addr, 0, stun_sever_addr, 0, &type))
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