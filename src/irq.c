#include "idt.h"
#include "kernel.h"

static void (*handlers[16])(void) = {0};

void irq_install_handler(int irq, void (*handler)(void)){
    if (irq >= 0 && irq < 16) handlers[irq] = handler;
}
void irq_uninstall_handler(int irq){
    if (irq >= 0 && irq < 16) handlers[irq] = 0;
}

void irq_handler(registers_t *r){
    int irq = r->int_no - 32;
    if (irq >= 0 && irq < 16 && handlers[irq]) handlers[irq]();
    if (irq >= 8) outb(0xA0, 0x20);
    outb(0x20, 0x20);
}
