#include "net.h"
#include "../kernel.h"

struct arp_hdr {
    uint16_t htype, ptype; uint8_t hlen, plen; uint16_t oper;
    uint8_t sha[6]; uint32_t spa; uint8_t tha[6]; uint32_t tpa;
} __attribute__((packed));

#define ARP_REQ 1
#define ARP_REP 2

static uint8_t cache_mac[6];
static uint32_t cache_ip = 0;
static int valid = 0;
static const uint8_t bcast[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

uint8_t *arp_resolve(uint32_t ip){
    if (valid && cache_ip == ip) return cache_mac;
    struct arp_hdr r;
    r.htype = htons(1); r.ptype = htons(ETHER_TYPE_IPV4);
    r.hlen = 6; r.plen = 4; r.oper = htons(ARP_REQ);
    memcpy(r.sha, iface.mac, 6);
    r.spa = htonl(iface.ip);
    memset(r.tha, 0, 6);
    r.tpa = htonl(ip);
    net_send_ethernet(bcast, ETHER_TYPE_ARP, (uint8_t*)&r, sizeof(r));
    return (uint8_t*)bcast;
}

void arp_handle(const uint8_t *pkt, uint16_t len){
    if (len < sizeof(struct arp_hdr)) return;
    struct arp_hdr *h = (struct arp_hdr*)pkt;
    if (htons(h->oper) == ARP_REQ && htonl(h->tpa) == iface.ip){
        struct arp_hdr rep;
        rep.htype = htons(1); rep.ptype = htons(ETHER_TYPE_IPV4);
        rep.hlen = 6; rep.plen = 4; rep.oper = htons(ARP_REP);
        memcpy(rep.sha, iface.mac, 6);
        rep.spa = htonl(iface.ip);
        memcpy(rep.tha, h->sha, 6);
        rep.tpa = h->spa;
        net_send_ethernet(h->sha, ETHER_TYPE_ARP, (uint8_t*)&rep, sizeof(rep));
        return;
    }
    if (htons(h->oper) == ARP_REP){
        memcpy(cache_mac, h->sha, 6);
        cache_ip = htonl(h->spa);
        valid = 1;
    }
}
