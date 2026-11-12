#include "net.h"
#include "../kernel.h"

struct icmp_hdr { uint8_t type, code; uint16_t csum; uint32_t rest; } __attribute__((packed));

void icmp_handle(const uint8_t *pkt, uint16_t len, uint32_t src_ip){
    if (len < sizeof(struct icmp_hdr)) return;
    struct icmp_hdr *h = (struct icmp_hdr*)pkt;
    if (h->type != 8) return;

    uint8_t echo[512];
    memcpy(echo, pkt, len);
    struct icmp_hdr *rh = (struct icmp_hdr*)echo;
    rh->type = 0; rh->code = 0; rh->csum = 0;
    rh->csum = checksum(echo, len);

    char ipb[16]; ip_to_str(src_ip, ipb);
    kprintf("[icmp] ping from %s\n", ipb);
    ipv4_send(NULL, PROTO_ICMP, iface.ip, src_ip, echo, len);
}
