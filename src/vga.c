#include "vga.h"
#include "kernel.h"
#include <stdarg.h>

#define VGA 0xB8000
#define COLS 80
#define ROWS 25

static uint16_t *vram = (uint16_t*)VGA;
static int row = 0, col = 0;

static inline uint16_t make(char c, uint8_t color){ return (uint16_t)c | ((uint16_t)color << 8); }

void vga_init(void){ row = col = 0; }
void vga_clear(void){
    for (int i = 0; i < COLS*ROWS; i++) vram[i] = make(' ', 0x0F);
    row = col = 0;
}
static void scroll(void){
    for (int r = 0; r < ROWS-1; r++)
        for (int c = 0; c < COLS; c++)
            vram[r*COLS+c] = vram[(r+1)*COLS+c];
    for (int c = 0; c < COLS; c++) vram[(ROWS-1)*COLS+c] = make(' ', 0x0F);
    row = ROWS-1;
}
void vga_putc(char c){
    if (c == '\n'){ col = 0; row++; }
    else if (c == '\r'){ col = 0; }
    else { vram[row*COLS+col] = make(c, 0x0F); col++; }
    if (col >= COLS){ col = 0; row++; }
    if (row >= ROWS) scroll();
}
void vga_puts(const char *s){ while (*s) vga_putc(*s++); }

static void put_uint(unsigned v, int base){ char b[34]; itoa((int)v, b, base); vga_puts(b); }

void kprintf(const char *fmt, ...){
    va_list ap; va_start(ap, fmt);
    for (const char *p = fmt; *p; p++){
        if (*p != '%'){ vga_putc(*p); continue; }
        p++;
        switch (*p){
            case 's': vga_puts(va_arg(ap, const char*)); break;
            case 'c': vga_putc((char)va_arg(ap, int)); break;
            case 'd': { int v = va_arg(ap, int); if (v < 0){ vga_putc('-'); put_uint((unsigned)(-v),10);} else put_uint((unsigned)v,10); break; }
            case 'u': put_uint(va_arg(ap, unsigned), 10); break;
            case 'x': put_uint(va_arg(ap, unsigned), 16); break;
            case '%': vga_putc('%'); break;
            default: vga_putc(*p); break;
        }
    }
    va_end(ap);
}
