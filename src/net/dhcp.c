#include "net.h"
#include "../kernel.h"

struct dhcp_hdr {
    uint8_t op, htype, hlen, hops;
    uint32_t xid;
    uint16_t secs, flags;
    uint32_t ciaddr, yiaddr, siaddr, giaddr;
    uint8_t chaddr[16];
    uint8_t sname[64];
    uint8_t file[128];
    uint32_t magic;
    uint8_t options[];
} __attribute__((packed));

#define DHCP_MAGIC 0x63825363
#define O_SUBNET 1
#define O_ROUTER 3
#define O_DNS 6
#define O_MSG 53
#define O_SERVER 54

static uint32_t xid;
static int done = 0;
static uint8_t pktbuf[576];

int dhcp_done(void){ return done; }

static uint16_t build(uint8_t type){
    struct dhcp_hdr *h = (struct dhcp_hdr*)pktbuf;
    memset(h, 0, sizeof(struct dhcp_hdr));
    h->op = 1; h->htype = 1; h->hlen = 6;
    h->xid = xid;
    memcpy(h->chaddr, iface.mac, 6);
    h->magic = htonl(DHCP_MAGIC);
    uint8_t *o = h->options;
    *o++ = O_MSG; *o++ = 1; *o++ = type;
    *o++ = 61; *o++ = 7; *o++ = 1;
    for (int i = 0; i < 6; i++) *o++ = iface.mac[i];
    *o++ = 55; *o++ = 3; *o++ = 1; *o++ = 3; *o++ = 6;
    *o++ = 255;
    return (uint16_t)((uint8_t*)o - (uint8_t*)h);
}

static void dhcp_send(uint8_t type){
    uint16_t paylen = build(type);
    struct { uint16_t sport, dport, len, csum; } __attribute__((packed)) u;
    u.sport = htons(68);
    u.dport = htons(67);
    u.len = htons((uint16_t)(8 + paylen));
    u.csum = 0;
    uint8_t frame[600];
    memcpy(frame, &u, 8);
    memcpy(frame + 8, pktbuf, paylen);
    uint8_t bmac[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    ipv4_send(bmac, PROTO_UDP, 0, 0xFFFFFFFF, frame, (uint16_t)(8 + paylen));
}

void dhcp_start(void){
    xid = 0x12345678;
    dhcp_send(1);
    kprintf("[dhcp] DISCOVER sent\n");
}

void dhcp_handle_packet(const uint8_t *d, uint16_t len, uint32_t src_ip){
    if (len < sizeof(struct dhcp_hdr)) return;
    struct dhcp_hdr *h = (struct dhcp_hdr*)d;
    if (htonl(h->magic) != DHCP_MAGIC) return;
    if (h->xid != xid) return;
    if (h->op != 2) return;

    uint8_t msg = 0;
    uint32_t mask = 0, router = 0, dns = 0;
    const uint8_t *o = h->options;
    const uint8_t *end = d + len;
    while (o + 1 < end && *o != 255){
        uint8_t code = *o++;
        if (code == 0) continue;
        uint8_t l = *o++;
        if (code == O_MSG) msg = o[0];
        else if (code == O_SUBNET) mask = ((uint32_t)o[0]<<24)|(o[1]<<16)|(o[2]<<8)|o[3];
        else if (code == O_ROUTER) router = ((uint32_t)o[0]<<24)|(o[1]<<16)|(o[2]<<8)|o[3];
        else if (code == O_DNS) dns = ((uint32_t)o[0]<<24)|(o[1]<<16)|(o[2]<<8)|o[3];
        o += l;
    }

    char ipb[16];
    if (msg == 2){ /* OFFER */
        iface.ip = htonl(h->yiaddr);
        if (mask) iface.netmask = mask;
        if (router) iface.gateway = router; else iface.gateway = src_ip;
        ip_to_str(iface.ip, ipb);
        kprintf("[dhcp] OFFER ip=%s\n", ipb);
        dhcp_send(3);
    } else if (msg == 5){ /* ACK */
        if (mask) iface.netmask = mask;
        if (router) iface.gateway = router;
        if (dns) iface.dns = dns; else iface.dns = iface.gateway;
        done = 1;
        ip_to_str(iface.ip, ipb);
        kprintf("[dhcp] ACK ip=%s\n", ipb);
        ip_to_str(iface.gateway, ipb);
        kprintf("[dhcp] gateway=%s\n", ipb);
        ip_to_str(iface.dns, ipb);
        kprintf("[dhcp] dns=%s\n", ipb);
    }
}
