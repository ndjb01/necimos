#include "net.h"
#include "../kernel.h"
#include "../drivers/rtl8139.h"

struct net_iface iface;

uint16_t htons(uint16_t x){ return (uint16_t)((x >> 8) | (x << 8)); }
uint32_t htonl(uint32_t x){
    return ((x & 0x000000FF) << 24) |
           ((x & 0x0000FF00) << 8)  |
           ((x & 0x00FF0000) >> 8)  |
           ((x & 0xFF000000) >> 24);
}

uint16_t checksum(const uint8_t *data, uint16_t len){
    uint32_t sum = 0;
    while (len > 1){ sum += ((uint16_t)data[0] << 8) | data[1]; data += 2; len -= 2; }
    if (len) sum += (uint16_t)data[0] << 8;
    while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
    return (uint16_t)~sum;
}

void ip_to_str(uint32_t ip, char *buf){
    int off = 0;
    for (int i = 24; i >= 0; i -= 8){
        if (off) buf[off++] = '.';
        char t[4]; itoa((int)((ip >> i) & 0xFF), t, 10);
        for (int k = 0; t[k]; k++) buf[off++] = t[k];
    }
    buf[off] = 0;
}

int net_init(void){
    rtl8139_init();
    for (int i = 0; i < 6; i++) iface.mac[i] = mac[i];
    int zero = 1;
    for (int i = 0; i < 6; i++) if (mac[i]) zero = 0;
    return zero ? -1 : 0;
}

void net_send_ethernet(const uint8_t *dst_mac, uint16_t ethertype,
                       const uint8_t *payload, uint16_t len){
    uint8_t frame[1520];
    memcpy(frame, dst_mac, 6);
    memcpy(frame + 6, iface.mac, 6);
    frame[12] = (uint8_t)(ethertype >> 8);
    frame[13] = (uint8_t)(ethertype & 0xFF);
    memcpy(frame + 14, payload, len);
    rtl8139_send(frame, (uint16_t)(len + 14));
}

void ipv4_send(const uint8_t *dst_mac, uint8_t proto, uint32_t src_ip,
               uint32_t dst_ip, const uint8_t *payload, uint16_t paylen){
    uint8_t out[1520];
    struct ip_hdr {
        uint8_t vihl, tos; uint16_t tot, id, frag; uint8_t ttl, proto; uint16_t csum; uint32_t src, dst;
    } __attribute__((packed)) *ip = (void*)out;
    ip->vihl = 0x45; ip->tos = 0;
    ip->tot = htons((uint16_t)(20 + paylen));
    ip->id = htons(1); ip->frag = 0;
    ip->ttl = 64; ip->proto = proto;
    ip->src = htonl(src_ip);
    ip->dst = htonl(dst_ip);
    ip->csum = 0;
    ip->csum = checksum(out, 20);
    memcpy(out + 20, payload, paylen);

    const uint8_t *m = dst_mac;
    if (!m){
        if ((dst_ip & iface.netmask) == (iface.ip & iface.netmask))
            m = arp_resolve(dst_ip);
        else
            m = arp_resolve(iface.gateway);
    }
    net_send_ethernet(m, ETHER_TYPE_IPV4, out, (uint16_t)(20 + paylen));
}

void udp_send(uint32_t src_ip, uint16_t sport, uint32_t dst_ip,
              uint16_t dport, const uint8_t *data, uint16_t len){
    struct { uint16_t sport, dport, len, csum; } __attribute__((packed)) u;
    u.sport = htons(sport);
    u.dport = htons(dport);
    u.len = htons((uint16_t)(8 + len));
    u.csum = 0;
    uint8_t pkt[1520];
    memcpy(pkt, &u, 8);
    if (data && len) memcpy(pkt + 8, data, len);
    ipv4_send(NULL, PROTO_UDP, src_ip, dst_ip, pkt, (uint16_t)(8 + len));
}

void net_poll(void){
    uint8_t buf[1600];
    uint16_t n;
    while ((n = rtl8139_receive(buf, sizeof(buf))) > 0)
        ethernet_handle(buf, n);
}
