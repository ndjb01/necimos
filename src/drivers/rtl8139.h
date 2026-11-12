#ifndef RTL8139_H
#define RTL8139_H
#include <stdint.h>
void rtl8139_init(void);
void rtl8139_send(const uint8_t *data, uint16_t len);
uint16_t rtl8139_receive(uint8_t *buf, uint16_t maxlen);
extern uint8_t mac[6];
#endif
