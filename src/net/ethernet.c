#include "net.h"

struct eth_hdr { uint8_t dst[6], src[6]; uint16_t type; } __attribute__((packed));

void ethernet_handle(const uint8_t *pkt, uint16_t len){
    if (len < 14) return;
    struct eth_hdr *h = (struct eth_hdr*)pkt;
    uint16_t t = htons(h->type);
    if (t == ETHER_TYPE_ARP) arp_handle(pkt + 14, len - 14);
    else if (t == ETHER_TYPE_IPV4) ipv4_handle(pkt + 14, len - 14);
}
