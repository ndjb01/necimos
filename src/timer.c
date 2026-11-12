#include "timer.h"
#include "kernel.h"
#include "idt.h"

volatile uint32_t ticks = 0;

static void tick_cb(void){ ticks++; }

void timer_init(uint32_t freq){
    uint32_t div = 1193180 / freq;
    outb(0x43, 0x36);
    outb(0x40, div & 0xFF);
    outb(0x40, (div >> 8) & 0xFF);
    irq_install_handler(0, tick_cb);
}
