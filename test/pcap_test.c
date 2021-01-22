
#include <stdio.h>
#include <pcap/pcap.h>      

void get_packet_cb(u_char *arg, const struct pcap_pkthdr *pkthdr, const u_char *packet)
{
      printf("Packet length: %d\n", pkthdr->len);
}

int main()      
{      
      
      char err_msg[PCAP_ERRBUF_SIZE] = {0};
      pcap_t *pcap_handler = pcap_open_live("vmnet8", 65535, 1, 100, err_msg);

      if(pcap_handler == NULL){
            printf("[ERROR] %s\n", err_msg);
            return (-1);
      }
      struct bpf_program filter;
      if(pcap_compile(pcap_handler, &filter, "ip proto gre", 1, 0) == -1){
            pcap_perror(pcap_handler, "[ERROR] Compile the expression of filter failed");
            goto _close;
      }
      pcap_setfilter(pcap_handler, &filter);

      int id = 0;
      
      pcap_loop(pcap_handler, -1, get_packet_cb, NULL);

      
      _close:pcap_close(pcap_handler);
      return 0;
}