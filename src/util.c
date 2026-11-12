#include "kernel.h"

void *memset(void *s, int c, size_t n){
    unsigned char *p = s;
    while (n--) *p++ = (unsigned char)c;
    return s;
}
void *memcpy(void *d, const void *s, size_t n){
    unsigned char *pd = d; const unsigned char *ps = s;
    while (n--) *pd++ = *ps++;
    return d;
}
void *memmove(void *d, const void *s, size_t n){
    unsigned char *pd = d; const unsigned char *ps = s;
    if (pd < ps){ while (n--) *pd++ = *ps++; }
    else { pd += n; ps += n; while (n--) *--pd = *--ps; }
    return d;
}
int memcmp(const void *a, const void *b, size_t n){
    const unsigned char *pa = a, *pb = b;
    while (n--){ if (*pa != *pb) return *pa - *pb; pa++; pb++; }
    return 0;
}
size_t strlen(const char *s){ size_t n = 0; while (s[n]) n++; return n; }
char *strcpy(char *d, const char *s){ char *o = d; while ((*d++ = *s++)); return o; }
char *strcat(char *d, const char *s){ char *o = d; while (*d) d++; while ((*d++ = *s++)); return o; }
char *itoa(int value, char *buf, int base){
    char tmp[34]; char *p = tmp; unsigned v; int neg = 0;
    if (base == 10 && value < 0){ neg = 1; v = (unsigned)(-value); } else v = (unsigned)value;
    do { int dg = v % base; *p++ = (dg < 10) ? '0'+dg : 'a'+dg-10; v /= base; } while (v);
    char *o = buf; if (neg) *o++ = '-';
    while (p > tmp) *o++ = *--p;
    *o = 0; return buf;
}
