#include "net.h"

struct udp_hdr { uint16_t sport, dport, len, csum; } __attribute__((packed));

void udp_handle(const uint8_t *pkt, uint16_t len, uint32_t src_ip){
    if (len < sizeof(struct udp_hdr)) return;
    struct udp_hdr *h = (struct udp_hdr*)pkt;
    uint16_t dp = htons(h->dport);
    if (dp == 68) dhcp_handle_packet(pkt + 8, len - 8, src_ip);
    else if (dp == 5353) dns_handle_packet(pkt + 8, len - 8);
}
