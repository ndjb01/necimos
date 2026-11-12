#include "rtl8139.h"
#include "../kernel.h"
#include "../pci.h"
#include "../idt.h"

#define VENDOR 0x10EC
#define DEVICE 0x8139

uint8_t mac[6];
static uint32_t io;
static uint8_t rx_buf[8192 + 16];
static uint32_t rx_cur;
static uint8_t tx_buf[2048];

static void rtl_irq(void){
    inw(io + 0x3E);
    outw(io + 0x3E, 0xFFFF);
}

void rtl8139_init(void){
    uint32_t loc = pci_find_device(VENDOR, DEVICE);
    if (loc == 0xFFFFFFFF) return;
    uint8_t bus = (loc >> 8) & 0xFF;
    uint8_t slot = (loc >> 3) & 0x1F;

    io = pci_read(bus, slot, 0, 0x10) & 0xFFFFFFFC;

    uint32_t cmd = pci_read(bus, slot, 0, 0x04);
    pci_write(bus, slot, 0, 0x04, cmd | 0x04);

    outb(io + 0x52, 0x00);
    outb(io + 0x37, 0x10);
    for (volatile int i = 0; i < 100000; i++) {}

    for (int i = 0; i < 6; i++) mac[i] = inb(io + i);

    rx_cur = 0;
    outl(io + 0x30, (uint32_t)rx_buf);
    outb(io + 0x37, 0x0C);

    uint8_t irq = pci_get_irq(bus, slot);
    if (irq >= 16) irq = 11;
    outw(io + 0x3C, 0x0005);
    irq_install_handler(irq, rtl_irq);
}

void rtl8139_send(const uint8_t *data, uint16_t len){
    if (len > sizeof(tx_buf)) return;
    memcpy(tx_buf, data, len);
    outl(io + 0x20, (uint32_t)tx_buf);
    outl(io + 0x10, (uint32_t)len & 0xFFFF);
}

uint16_t rtl8139_receive(uint8_t *buf, uint16_t maxlen){
    uint8_t cmd = inb(io + 0x37);
    if (!(cmd & 0x01)) return 0;

    uint16_t off = rx_cur % 8192;
    uint8_t *p = rx_buf + off;
    uint16_t status = p[0] | (p[1] << 8);
    uint16_t len = p[2] | (p[3] << 8);

    if (!(status & 0x0001) || (status & 0x1E) || len == 0 || len > maxlen){
        outb(io + 0x37, 0x04);
        outb(io + 0x37, 0x0C);
        rx_cur = 0;
        return 0;
    }

    memcpy(buf, p + 4, len);
    rx_cur = (rx_cur + len + 4 + 3) & ~3;
    outw(io + 0x38, rx_cur - 16);
    return len;
}
