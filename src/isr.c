#include "idt.h"
#include "kernel.h"
#include "vga.h"

static const char *names[] = {
    "Divide by zero","Debug","NMI","Breakpoint","Overflow","Bound range",
    "Invalid opcode","Device not available","Double fault","Coprocessor seg",
    "Invalid TSS","Segment not present","Stack fault","GPF","Page fault",
    "Reserved","x87 FPU error","Alignment check","Machine check","SIMD FPU",
    "Virtualization","Reserved","Reserved","Reserved","Reserved","Reserved",
    "Reserved","Reserved","Reserved","Reserved","Security exception","Reserved"
};

void isr_handler(registers_t *r){
    const char *name = (r->int_no < 32) ? names[r->int_no] : "unknown";
    kprintf("\n!! EXCEPTION %d (%s) err=%x\nSystem halted.\n", r->int_no, name, r->err);
    for (;;) hlt();
}
