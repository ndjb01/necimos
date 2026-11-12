#include "net.h"
#include "../kernel.h"
#include "../timer.h"

struct dns_hdr {
    uint16_t id, flags, qdcount, ancount, nscount, arcount;
} __attribute__((packed));

static uint16_t dns_id = 0;
static uint32_t answer = 0;
static int answered = 0;

void dns_start(const char *name){
    const char *log = name;
    uint8_t q[512];
    struct dns_hdr *h = (struct dns_hdr*)q;
    dns_id = (uint16_t)(0x4000 + (ticks & 0x3FFF));
    h->id = htons(dns_id);
    h->flags = htons(0x0100);
    h->qdcount = htons(1);
    h->ancount = htons(0);
    h->nscount = htons(0);
    h->arcount = htons(0);
    uint16_t off = sizeof(struct dns_hdr);
    while (*name){
        const char *e = name;
        while (*e && *e != '.') e++;
        uint8_t l = (uint8_t)(e - name);
        q[off++] = l;
        for (const char *c = name; c < e; c++) q[off++] = (uint8_t)*c;
        name = (*e == '.') ? e + 1 : e;
    }
    q[off++] = 0;
    q[off++] = 0; q[off++] = 1;
    q[off++] = 0; q[off++] = 1;
    answered = 0;
    char ipb[16]; ip_to_str(iface.dns, ipb);
    kprintf("[dns] query %s -> %s\n", log, ipb);
    udp_send(iface.ip, 5353, iface.dns, 53, q, off);
}

void dns_handle_packet(const uint8_t *d, uint16_t len){
    if (len < sizeof(struct dns_hdr)) return;
    struct dns_hdr *h = (struct dns_hdr*)d;
    if (htons(h->id) != dns_id) return;
    uint16_t qd = htons(h->qdcount);
    uint16_t an = htons(h->ancount);
    uint16_t off = sizeof(struct dns_hdr);
    while (qd-- && off < len){
        while (off < len && d[off] != 0) off++;
        off += 5;
    }
    while (an-- && off < len){
        if (off + 12 > len) break;
        if (d[off] & 0xC0) off += 2;
        else { while (off < len && d[off] != 0) off++; off += 1; }
        uint16_t type = (d[off] << 8) | d[off + 1];
        uint16_t cls  = (d[off + 2] << 8) | d[off + 3];
        uint16_t rdlen = (d[off + 8] << 8) | d[off + 9];
        off += 10;
        if (type == 1 && cls == 1 && rdlen == 4 && off + 4 <= len){
            answer = ((uint32_t)d[off] << 24) | ((uint32_t)d[off+1] << 16) |
                     ((uint32_t)d[off+2] << 8) | d[off+3];
            answered = 1;
            char ipb[16]; ip_to_str(answer, ipb);
            kprintf("[dns] resolved -> %s\n", ipb);
            return;
        }
        off += rdlen;
    }
}

int dns_poll(void){ return answered; }
uint32_t dns_get_ip(void){ return answer; }
