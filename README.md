# necimos

Bootable x86 hobby operating system with a real TCP/IP network stack.

## Features
- Multiboot kernel (GRUB) in 32-bit protected mode
- GDT, IDT, PIC remap, ISR/IRQ, PIT timer, PS/2 keyboard
- VGA text console with printf
- PCI enumeration
- RTL8139 NIC driver (emulated by QEMU)
- Ethernet / ARP / IPv4 / ICMP (ping) / UDP
- DHCP client (auto IP, netmask, gateway, DNS)
- TCP client, DNS resolver (A records), HTTP GET client

Boot sequence: DHCP -> resolve example.com via DNS -> open TCP connection -> GET the page -> print it.

## Build
Requires: nasm, xorriso, grub-pc-bin (grub-mkrescue), qemu-system-i386, and an
i686-elf cross-compiler (https://wiki.osdev.org/GCC_Cross-Compiler).

    make iso
    make run

`make run` boots QEMU with an RTL8139 NIC on usermode networking.
