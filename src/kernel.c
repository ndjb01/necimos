#include "kernel.h"
#include "vga.h"
#include "gdt.h"
#include "idt.h"
#include "timer.h"
#include "keyboard.h"
#include "net/net.h"

#define HOST "example.com"
#define PATH "/"

static int phase = 0;

void kernel_main(void){
    vga_init();
    vga_clear();
    vga_puts("necimos v0.1 - boot\n");

    gdt_init();      vga_puts("[ok] GDT\n");
    idt_init();      vga_puts("[ok] IDT+PIC\n");
    timer_init(100); vga_puts("[ok] PIT 100Hz\n");
    keyboard_init(); vga_puts("[ok] keyboard\n");
    sti();

    if (net_init() == 0){ vga_puts("[ok] RTL8139\n"); dhcp_start(); }
    else vga_puts("[!!] no NIC found\n");

    for (;;){
        net_poll();
        tcp_tick();

        if (phase == 0 && dhcp_done()){ phase = 1; dns_start(HOST); }
        else if (phase == 1 && dns_poll()){ phase = 2; http_start(dns_get_ip(), HOST, PATH); }
        else if (phase == 2 && http_poll()){ phase = 3; }

        hlt();
    }
}
