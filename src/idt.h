#ifndef IDT_H
#define IDT_H
#include <stdint.h>

typedef struct {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp_val, ebx, edx, ecx, eax;
    uint32_t int_no, err;
} registers_t;

void idt_init(void);
void irq_install_handler(int irq, void (*handler)(void));
void irq_uninstall_handler(int irq);
#endif
