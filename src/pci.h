#ifndef PCI_H
#define PCI_H
#include <stdint.h>
uint32_t pci_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void pci_write(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t val);
uint32_t pci_find_device(uint16_t vendor, uint16_t device);
uint8_t pci_get_irq(uint8_t bus, uint8_t slot);
#endif
