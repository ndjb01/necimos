#ifndef VGA_H
#define VGA_H
#include <stdint.h>
void vga_init(void);
void vga_clear(void);
void vga_putc(char c);
void vga_puts(const char *s);
void kprintf(const char *fmt, ...);
#endif
