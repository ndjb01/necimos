#include "net.h"

struct ip_hdr {
    uint8_t vihl, tos; uint16_t tot, id, frag; uint8_t ttl, proto; uint16_t csum; uint32_t src, dst;
} __attribute__((packed));

void ipv4_handle(const uint8_t *pkt, uint16_t len){
    if (len < sizeof(struct ip_hdr)) return;
    struct ip_hdr *h = (struct ip_hdr*)pkt;
    if ((h->vihl >> 4) != 4) return;
    if (htonl(h->dst) != iface.ip) return;
    uint16_t ihl = (h->vihl & 0x0F) * 4;
    uint32_t src = htonl(h->src);
    if (h->proto == PROTO_ICMP) icmp_handle(pkt + ihl, len - ihl, src);
    else if (h->proto == PROTO_UDP) udp_handle(pkt + ihl, len - ihl, src);
    else if (h->proto == PROTO_TCP) tcp_handle_segment(pkt + ihl, len - ihl, src);
}
