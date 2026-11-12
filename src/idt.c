#include "idt.h"
#include "kernel.h"

struct idt_entry {
    uint16_t base_low;
    uint16_t sel;
    uint8_t zero;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct idt_entry idt[256];
static struct idt_ptr idtp;

extern void idt_flush(uint32_t);
extern uint32_t isr_stub_table[];
extern uint32_t irq_stub_table[];

static void set_gate(int n, uint32_t handler, uint16_t sel, uint8_t flags){
    idt[n].base_low = handler & 0xFFFF;
    idt[n].base_high = (handler >> 16) & 0xFFFF;
    idt[n].sel = sel;
    idt[n].zero = 0;
    idt[n].flags = flags;
}

void idt_init(void){
    idtp.limit = sizeof(idt) - 1;
    idtp.base = (uint32_t)&idt;
    memset(&idt, 0, sizeof(idt));

    for (int i = 0; i < 32; i++)
        set_gate(i, isr_stub_table[i], 0x08, 0x8E);

    outb(0x20, 0x11); outb(0xA0, 0x11);
    outb(0x21, 0x20); outb(0xA1, 0x28);
    outb(0x21, 0x04); outb(0xA1, 0x02);
    outb(0x21, 0x01); outb(0xA1, 0x01);
    outb(0x21, 0x00); outb(0xA1, 0x00);

    for (int i = 0; i < 16; i++)
        set_gate(32 + i, irq_stub_table[i], 0x08, 0x8E);

    idt_flush((uint32_t)&idtp);
}
