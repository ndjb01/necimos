#ifndef KERNEL_H
#define KERNEL_H
#include <stdint.h>
#include <stddef.h>

static inline void outb(uint16_t port, uint8_t val){ __asm__ volatile("outb %0,%1"::"a"(val),"Nd"(port)); }
static inline uint8_t inb(uint16_t port){ uint8_t r; __asm__ volatile("inb %1,%0":"=a"(r):"Nd"(port)); return r; }
static inline void outw(uint16_t port, uint16_t val){ __asm__ volatile("outw %0,%1"::"a"(val),"Nd"(port)); }
static inline uint16_t inw(uint16_t port){ uint16_t r; __asm__ volatile("inw %1,%0":"=a"(r):"Nd"(port)); return r; }
static inline void outl(uint16_t port, uint32_t val){ __asm__ volatile("outl %0,%1"::"a"(val),"Nd"(port)); }
static inline uint32_t inl(uint16_t port){ uint32_t r; __asm__ volatile("inl %1,%0":"=a"(r):"Nd"(port)); return r; }

static inline void sti(void){ __asm__ volatile("sti"); }
static inline void cli(void){ __asm__ volatile("cli"); }
static inline void hlt(void){ __asm__ volatile("hlt"); }

void *memset(void *s, int c, size_t n);
void *memcpy(void *dst, const void *src, size_t n);
void *memmove(void *dst, const void *src, size_t n);
int   memcmp(const void *a, const void *b, size_t n);
size_t strlen(const char *s);
char *strcpy(char *d, const char *s);
char *strcat(char *d, const char *s);
char *itoa(int value, char *buf, int base);

#endif
