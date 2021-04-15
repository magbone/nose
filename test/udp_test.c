#include "../src/udp.h"
#include "../src/pmp.h"

int main()
{
      struct udp_handler handler;

      struct bucket_item items[2] = {{0}};

      items[0].port = 9998;
      memcpy(items[0].ipv4, "192.168.0.1", strlen("192.168.0.1"));
      memcpy(items[0].node_id, "bbbbbbbbbbbbbbbbbbbb", 20);

      items[1].port = 9998;
      memcpy(items[1].ipv4, "192.168.1.1", strlen("192.168.1.1"));
      memcpy(items[1].node_id, "cccccbbbbbbbbbbbbbbb", 20);

      char buf[1024];
      int len = PMP_discovery_req_pkt(9998, "aaaaaaaaaaaaaaaaaaaa", "bbbbbbbbbbbbbbbbbbbb", buf);
      send_udp_pkt(&handler, "127.0.0.1", 9998, 5, buf, len);
      len = PMP_ping_req_pkt("aaaaaaaaaaaaaaaaaaaa", "bbbbbbbbbbbbbbbbbbbb", buf);
      send_udp_pkt(&handler, "127.0.0.1", 9998, 5, buf, len);
      len = PMP_discovery_rsp_pkt("bbbbbbbbbbbbbbbbbbbb", items, 2, buf);
      send_udp_pkt(&handler, "127.0.0.1", 9998, 5, buf, len);
      recv_udp_pkt(&handler, buf);
      return 0;
}