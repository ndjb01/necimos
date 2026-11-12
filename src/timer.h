#ifndef TIMER_H
#define TIMER_H
#include <stdint.h>
extern volatile uint32_t ticks;
void timer_init(uint32_t freq);
#endif
