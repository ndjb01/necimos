#include "net.h"
#include "../kernel.h"
#include "../timer.h"

struct tcp_hdr {
    uint16_t sport, dport;
    uint32_t seq, ack;
    uint8_t dataoff;
    uint8_t flags;
    uint16_t window, checksum, urg;
} __attribute__((packed));

#define F_FIN 0x01
#define F_SYN 0x02
#define F_RST 0x04
#define F_PSH 0x08
#define F_ACK 0x10

enum { ST_CLOSED, ST_SYN_SENT, ST_ESTABLISHED, ST_FIN_WAIT1, ST_FIN_WAIT2, ST_TIME_WAIT };

static uint8_t  state = ST_CLOSED;
static uint16_t lport = 12345;
static uint32_t rip, rport;
static uint32_t iss, snd_una, snd_nxt, rcv_nxt;

static uint8_t  rx[8192];
static uint16_t rx_len = 0;

static uint32_t rtx_seq, rtx_time;
static uint8_t  rtx_data[2048];
static uint16_t rtx_len = 0;
static uint8_t  rtx_flags = 0;
static int       rtx_active = 0;

static void tcp_tx(uint8_t flags, uint32_t seq, uint32_t ack,
                   const uint8_t *data, uint16_t dlen){
    struct tcp_hdr h;
    h.sport = htons(lport);
    h.dport = htons((uint16_t)rport);
    h.seq = htonl(seq);
    h.ack = htonl(ack);
    h.dataoff = 0x50;
    h.flags = flags;
    h.window = htons(4096);
    h.checksum = 0;
    h.urg = 0;

    uint8_t seg[2100];
    memcpy(seg, &h, sizeof(h));
    if (data && dlen) memcpy(seg + sizeof(h), data, dlen);

    uint8_t ph[12];
    *(uint32_t*)(ph + 0) = htonl(iface.ip);
    *(uint32_t*)(ph + 4) = htonl(rip);
    ph[8] = 0; ph[9] = PROTO_TCP;
    *(uint16_t*)(ph + 10) = htons((uint16_t)(sizeof(h) + dlen));

    uint8_t cbuf[2112];
    memcpy(cbuf, ph, 12);
    memcpy(cbuf + 12, seg, sizeof(h) + dlen);
    h.checksum = checksum(cbuf, (uint16_t)(12 + sizeof(h) + dlen));
    memcpy(seg, &h, sizeof(h));

    rtx_seq = seq;
    rtx_flags = flags;
    rtx_len = (data && dlen <= sizeof(rtx_data)) ? dlen : 0;
    if (rtx_len) memcpy(rtx_data, data, dlen);
    rtx_active = (flags & (F_SYN | F_FIN)) || rtx_len;
    rtx_time = ticks;

    ipv4_send(NULL, PROTO_TCP, iface.ip, rip, seg, (uint16_t)(sizeof(h) + dlen));
}

int tcp_connect(uint32_t dst_ip, uint16_t dst_port){
    if (state != ST_CLOSED) return -1;
    rip = dst_ip; rport = dst_port;
    iss = 0x1000 + (ticks & 0xFFFF);
    snd_una = iss; snd_nxt = iss;
    state = ST_SYN_SENT;
    char ipb[16]; ip_to_str(rip, ipb);
    kprintf("[tcp] SYN -> %s:%d\n", ipb, rport);
    tcp_tx(F_SYN, iss, 0, NULL, 0);
    return 0;
}

int tcp_send(const uint8_t *data, uint16_t len){
    if (state != ST_ESTABLISHED) return -1;
    tcp_tx(F_ACK | F_PSH, snd_nxt, rcv_nxt, data, len);
    snd_nxt += len;
    return len;
}

int tcp_receive(uint8_t *buf, uint16_t maxlen){
    if (!rx_len) return 0;
    uint16_t n = rx_len < maxlen ? rx_len : maxlen;
    memcpy(buf, rx, n);
    memmove(rx, rx + n, rx_len - n);
    rx_len -= n;
    return n;
}

int tcp_closed(void){ return state == ST_CLOSED || state == ST_TIME_WAIT; }

void tcp_handle_segment(const uint8_t *seg, uint16_t len, uint32_t src_ip){
    if (len < sizeof(struct tcp_hdr)) return;
    struct tcp_hdr *h = (struct tcp_hdr*)seg;
    if (htons(h->dport) != lport) return;

    uint8_t flags = h->flags;
    uint16_t hl = (h->dataoff >> 4) * 4;
    if (hl < sizeof(struct tcp_hdr)) return;
    const uint8_t *data = seg + hl;
    uint16_t dlen = len - hl;
    uint32_t seq = htonl(h->seq);
    uint32_t ack = htonl(h->ack);

    if (flags & F_RST){ state = ST_CLOSED; return; }

    if (state == ST_SYN_SENT){
        if ((flags & F_SYN) && (flags & F_ACK) && ack == (snd_una + 1)){
            rcv_nxt = seq + 1;
            snd_una = ack; snd_nxt = ack;
            state = ST_ESTABLISHED;
            kprintf("[tcp] connected\n");
            tcp_tx(F_ACK, snd_nxt, rcv_nxt, NULL, 0);
        }
        return;
    }

    if (state == ST_ESTABLISHED || state == ST_FIN_WAIT1 || state == ST_FIN_WAIT2){
        if (dlen && seq == rcv_nxt){
            if ((uint32_t)rx_len + dlen <= sizeof(rx)){
                memcpy(rx + rx_len, data, dlen);
                rx_len += dlen;
            }
            rcv_nxt += dlen;
            tcp_tx(F_ACK, snd_nxt, rcv_nxt, NULL, 0);
        }
        if (flags & F_ACK){
            if (ack > snd_una) snd_una = ack;
            if (ack >= snd_nxt) rtx_active = 0;
        }
        if (flags & F_FIN){
            rcv_nxt += 1;
            tcp_tx(F_ACK, snd_nxt, rcv_nxt, NULL, 0);
            state = ST_TIME_WAIT;
            kprintf("[tcp] FIN received\n");
        }
        return;
    }
}

void tcp_tick(void){
    if (!rtx_active) return;
    if (ticks - rtx_time < 50) return;
    if (state == ST_SYN_SENT) tcp_tx(F_SYN, iss, 0, NULL, 0);
    else if (rtx_len) tcp_tx(rtx_flags, rtx_seq, rcv_nxt, rtx_data, rtx_len);
    else if (rtx_flags & F_FIN) tcp_tx(rtx_flags, rtx_seq, rcv_nxt, NULL, 0);
    rtx_time = ticks;
}
