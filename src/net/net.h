#ifndef NET_H
#define NET_H
#include <stdint.h>

#define ETHER_TYPE_IPV4 0x0800
#define ETHER_TYPE_ARP  0x0806
#define PROTO_ICMP 1
#define PROTO_TCP  6
#define PROTO_UDP  17

struct net_iface {
    uint8_t mac[6];
    uint32_t ip, netmask, gateway, dns;
};

extern struct net_iface iface;
extern uint8_t mac[6];

int  net_init(void);
void net_poll(void);
void net_send_ethernet(const uint8_t *dst_mac, uint16_t ethertype,
                       const uint8_t *payload, uint16_t len);
void ipv4_send(const uint8_t *dst_mac, uint8_t proto, uint32_t src_ip,
               uint32_t dst_ip, const uint8_t *payload, uint16_t paylen);
void udp_send(uint32_t src_ip, uint16_t sport, uint32_t dst_ip,
              uint16_t dport, const uint8_t *data, uint16_t len);

uint8_t *arp_resolve(uint32_t ip);

void ethernet_handle(const uint8_t *pkt, uint16_t len);
void arp_handle(const uint8_t *pkt, uint16_t len);
void ipv4_handle(const uint8_t *pkt, uint16_t len);
void icmp_handle(const uint8_t *pkt, uint16_t len, uint32_t src_ip);
void udp_handle(const uint8_t *pkt, uint16_t len, uint32_t src_ip);
void tcp_handle_segment(const uint8_t *seg, uint16_t len, uint32_t src_ip);

int tcp_connect(uint32_t dst_ip, uint16_t dst_port);
int tcp_send(const uint8_t *data, uint16_t len);
int tcp_receive(uint8_t *buf, uint16_t maxlen);
int tcp_closed(void);
void tcp_tick(void);

void dns_start(const char *name);
void dns_handle_packet(const uint8_t *pkt, uint16_t len);
int  dns_poll(void);
uint32_t dns_get_ip(void);

void http_start(uint32_t ip, const char *host, const char *path);
int  http_poll(void);

void dhcp_start(void);
void dhcp_handle_packet(const uint8_t *pkt, uint16_t len, uint32_t src_ip);
int dhcp_done(void);

uint16_t checksum(const uint8_t *data, uint16_t len);
uint16_t htons(uint16_t x);
uint32_t htonl(uint32_t x);
void ip_to_str(uint32_t ip, char *buf);
#endif
