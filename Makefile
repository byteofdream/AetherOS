ARCH := x86_64
BUILD := build
ISO_ROOT := $(BUILD)/iso
KERNEL := $(BUILD)/kernel.elf
ISO := $(BUILD)/aetheros.iso

CC := gcc
LD := ld
NASM := nasm
OBJCOPY := objcopy
GRUB_MKRESCUE := grub-mkrescue
QEMU := qemu-system-x86_64

CFLAGS := -std=gnu11 -ffreestanding -fno-stack-protector -fno-pic -fno-pie -m64 -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -O2 -Wall -Wextra -I. -Ikernel/include -Idrivers/include -Ifs/include -Iuserland/include
LDFLAGS := -nostdlib -z max-page-size=0x1000
APP_CFLAGS := -std=gnu11 -ffreestanding -fno-stack-protector -fno-pic -fno-pie -m64 -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -O2 -Wall -Wextra -Iuserland/include
APP_LDFLAGS := -nostdlib -no-pie

KERNEL_C_SRCS := \
	kernel/main.c \
	kernel/console.c \
	kernel/graphics.c \
	kernel/string.c \
	kernel/port.c \
	kernel/multiboot.c \
	kernel/gdt.c \
	kernel/idt.c \
	kernel/interrupts.c \
	kernel/pic.c \
	kernel/pit.c \
	kernel/memory.c \
	kernel/heap.c \
	kernel/scheduler.c \
	kernel/wm.c \
	kernel/cpu.c \
	drivers/pci.c \
	drivers/usb.c \
	drivers/ps2.c \
	drivers/keyboard.c \
	drivers/mouse.c \
	drivers/ata.c \
	fs/ramfs.c \
	userland/elf.c \
	userland/shell.c

KERNEL_ASM_SRCS := \
	boot/boot.asm \
	kernel/interrupt_stubs.asm \
	kernel/context_switch.asm

KERNEL_C_OBJS := $(patsubst %.c,$(BUILD)/%.o,$(KERNEL_C_SRCS))
KERNEL_ASM_OBJS := $(patsubst %.asm,$(BUILD)/%.o,$(KERNEL_ASM_SRCS))

APP_SRCS := userland/apps/hello.c userland/apps/info.c userland/apps/sysinfo.c userland/apps/demo.cpp
APP_ELFS := $(BUILD)/userland/hello.elf $(BUILD)/userland/info.elf $(BUILD)/userland/sysinfo.elf $(BUILD)/userland/demo.elf
APP_BIN_OBJS := $(BUILD)/userland/hello_app.o $(BUILD)/userland/info_app.o $(BUILD)/userland/sysinfo_app.o $(BUILD)/userland/demo_app.o

.PHONY: all clean run iso

all: $(ISO)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: %.asm
	mkdir -p $(dir $@)
	$(NASM) -f elf64 $< -o $@

$(BUILD)/userland/%.o: userland/apps/%.c
	mkdir -p $(dir $@)
	$(CC) $(APP_CFLAGS) -c $< -o $@

$(BUILD)/userland/%.o: userland/apps/%.cpp
	mkdir -p $(dir $@)
	$(CC) $(APP_CFLAGS) -c $< -o $@

$(BUILD)/userland/hello.elf: $(BUILD)/userland/hello.o userland/apps/app.ld
	$(CC) $(APP_CFLAGS) $(APP_LDFLAGS) -Wl,-T,userland/apps/app.ld -Wl,-Ttext-segment=0x400000 -Wl,-e,app_entry $< -o $@

$(BUILD)/userland/info.elf: $(BUILD)/userland/info.o userland/apps/app.ld
	$(CC) $(APP_CFLAGS) $(APP_LDFLAGS) -Wl,-T,userland/apps/app.ld -Wl,-Ttext-segment=0x500000 -Wl,-e,app_entry $< -o $@

$(BUILD)/userland/sysinfo.elf: $(BUILD)/userland/sysinfo.o userland/apps/app.ld
	$(CC) $(APP_CFLAGS) $(APP_LDFLAGS) -Wl,-T,userland/apps/app.ld -Wl,-Ttext-segment=0x600000 -Wl,-e,app_entry $< -o $@

$(BUILD)/userland/demo.elf: $(BUILD)/userland/demo.o userland/apps/app.ld
	$(CC) $(APP_CFLAGS) $(APP_LDFLAGS) -Wl,-T,userland/apps/app.ld -Wl,-Ttext-segment=0x700000 -Wl,-e,app_entry $< -o $@

$(BUILD)/userland/%_app.o: $(BUILD)/userland/%.elf
	$(OBJCOPY) -I binary -O elf64-x86-64 -B i386:x86-64 $< $@

$(KERNEL): $(KERNEL_C_OBJS) $(KERNEL_ASM_OBJS) $(APP_ELFS) $(APP_BIN_OBJS) kernel/linker.ld
	$(LD) $(LDFLAGS) -T kernel/linker.ld -o $@ $(KERNEL_ASM_OBJS) $(KERNEL_C_OBJS) $(APP_BIN_OBJS)

$(ISO): $(KERNEL) boot/grub/grub.cfg
	mkdir -p $(ISO_ROOT)/boot/grub
	cp $(KERNEL) $(ISO_ROOT)/boot/kernel.elf
	cp boot/grub/grub.cfg $(ISO_ROOT)/boot/grub/grub.cfg
	$(GRUB_MKRESCUE) -o $@ $(ISO_ROOT)

iso: $(ISO)

run: $(ISO)
	$(QEMU) -cdrom $(ISO) -m 256M

clean:
	rm -rf $(BUILD)
