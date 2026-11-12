#include "pci.h"
#include "kernel.h"

uint32_t pci_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset){
    uint32_t addr = 0x80000000 | ((uint32_t)bus << 16) | ((uint32_t)slot << 11) |
                    ((uint32_t)func << 8) | (offset & 0xFC);
    outl(0xCF8, addr);
    return inl(0xCFC);
}

void pci_write(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t val){
    uint32_t addr = 0x80000000 | ((uint32_t)bus << 16) | ((uint32_t)slot << 11) |
                    ((uint32_t)func << 8) | (offset & 0xFC);
    outl(0xCF8, addr);
    outl(0xCFC, val);
}

uint32_t pci_find_device(uint16_t vendor, uint16_t device){
    for (uint16_t bus = 0; bus < 256; bus++){
        for (uint8_t slot = 0; slot < 32; slot++){
            uint32_t v = pci_read(bus, slot, 0, 0);
            if ((v & 0xFFFF) == 0xFFFF) continue;
            if ((v & 0xFFFF) == vendor){
                uint32_t d = pci_read(bus, slot, 0, 2);
                if (((d >> 16) & 0xFFFF) == device)
                    return ((uint32_t)bus << 8) | ((uint32_t)slot << 3);
            }
        }
    }
    return 0xFFFFFFFF;
}

uint8_t pci_get_irq(uint8_t bus, uint8_t slot){
    uint32_t v = pci_read(bus, slot, 0, 0x3C);
    return (uint8_t)(v & 0xFF);
}
