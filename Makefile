.RECIPEPREFIX = >

ARCH := i686-elf
CC  := $(ARCH)-gcc
AS  := nasm
LD  := $(ARCH)-ld

SRC_DIR   := src
BUILD_DIR := build

C_SOURCES   := $(shell find $(SRC_DIR) -name '*.c')
ASM_SOURCES := $(shell find $(SRC_DIR) -name '*.asm')
C_OBJECTS   := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SOURCES))
ASM_OBJECTS := $(patsubst $(SRC_DIR)/%.asm,$(BUILD_DIR)/%.o,$(ASM_SOURCES))

CFLAGS := -std=gnu99 -ffreestanding -O2 -Wall -Wextra -fno-stack-protector \
          -fno-pic -m32 -march=i686 -I$(SRC_DIR)

all: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
>@mkdir -p $(dir $@)
>$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.asm
>@mkdir -p $(dir $@)
>$(AS) -f elf32 $< -o $@

$(BUILD_DIR)/kernel.bin: $(C_OBJECTS) $(ASM_OBJECTS)
>$(LD) -T linker.ld -o $(BUILD_DIR)/kernel.elf $^
>cp $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/kernel.bin

iso: all
>@mkdir -p iso/boot/grub
>cp $(BUILD_DIR)/kernel.bin iso/boot/
>cp grub.cfg iso/boot/grub/
>grub-mkrescue -o necimos.iso iso

run: iso
>qemu-system-i386 -cdrom necimos.iso -net nic,model=rtl8139 -net user

clean:
>rm -rf $(BUILD_DIR) iso necimos.iso

.PHONY: all iso run clean
