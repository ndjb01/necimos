#include "keyboard.h"
#include "kernel.h"
#include "idt.h"

static void kb_handler(void){ (void)inb(0x60); }

void keyboard_init(void){ irq_install_handler(1, kb_handler); }
