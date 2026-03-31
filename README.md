# AetherOS

Minimal 64-bit OS loaded by GRUB via Multiboot2, now with a keyboard-only window manager and a tiny framebuffer graphics layer.

## What works

- GRUB/Multiboot2 boot path
- 32-bit bootstrap that switches to x86_64 long mode
- ELF64 kernel
- IDT and basic exception/IRQ dispatch
- PIC remap and PIT timer
- Simple cooperative scheduler with kernel tasks
- GRUB framebuffer graphics path
- Bitmap-font text rendering
- Offscreen backbuffer with present step to reduce flicker
- PS/2 keyboard IRQ handling
- PS/2 mouse packet decoding
- ATA PIO identify/read/write routines
- PCI scan
- Minimal USB host-controller detection layer
- In-memory RAM filesystem with read/write support
- ELF loader for simple freestanding user apps
- Keyboard-driven VGA window manager
- Theme-based visual styling for the WM
- Gaps between tiled windows and from screen edges
- Top status bar with active window, uptime clock, and window count
- Terminal window inside the WM
- `.lx` text app windows launched from the terminal

## Current limitations

- The graphics path currently expects a 32-bit GRUB framebuffer mode
- USB only detects UHCI/EHCI-class controllers; it does not enumerate devices yet
- Filesystem is RAM-backed, not FAT32 or disk-backed
- User apps run in the kernel address space without ring3 isolation
- Real-hardware support depends on GRUB providing a compatible framebuffer
- The WM is intentionally simple: no mouse, no compositing, and no overlapping windows
- Rendering stays CPU-light and software-rendered

## Project layout

- `boot/` - GRUB config and long-mode bootstrap
- `kernel/` - core kernel code
- `kernel/graphics.c` - framebuffer drawing, bitmap font, and backbuffer
- `kernel/wm.c` - simple keyboard-only window manager
- `drivers/` - PS/2, ATA, PCI, USB
- `fs/` - RAM filesystem
- `userland/` - ELF loader, shell, sample apps

## Build

Requirements:

- `gcc`
- `ld`
- `nasm`
- `grub-mkrescue`
- `xorriso`
- `qemu-system-x86_64`

Build the kernel and ISO:

```bash
make
```

Run in QEMU:

```bash
make run
```

Artifacts:

- `build/kernel.elf`
- `build/aetheros.iso`

## Boot flow

1. GRUB loads `kernel.elf` using the Multiboot2 header from `boot/boot.asm`.
2. The bootstrap code creates a minimal identity-mapped page-table hierarchy.
3. Paging and long mode are enabled.
4. Control jumps into 64-bit C code at `kernel_entry64`.
5. The kernel parses Multiboot tags, initializes memory/interrupts/drivers, mounts RAMFS, and starts the window-manager task.

## Interrupt handling

- `kernel/interrupt_stubs.asm` defines exception and IRQ stubs.
- Each stub saves general-purpose registers and calls `interrupt_dispatch`.
- `interrupt_dispatch` routes exceptions and hardware IRQs.
- PIC EOIs are sent after IRQ handlers run.
- PIT IRQ0 increments the global tick counter.

## Keyboard and mouse drivers

### PS/2 keyboard

- The PS/2 controller is enabled through ports `0x64` and `0x60`.
- IRQ1 reads a scan code from port `0x60`.
- The driver tracks `Alt`, `Shift`, arrow keys, `Tab`, `Enter`, and ASCII keys.
- The window manager consumes structured key events from this queue.

### PS/2 mouse

- The kernel enables the auxiliary PS/2 device and sends the `0xF4` streaming command.
- IRQ12 collects 3-byte mouse packets.
- Packets are decoded into `x`, `y`, and button state.

## USB

- `drivers/usb.c` performs a minimal PCI scan for class `0x0C`, subclass `0x03`.
- This is enough to detect USB host controllers and print their basic identity.
- Full enumeration, transfers, and HID/storage class support are not implemented yet.

## Filesystem and user apps

- `fs/ramfs.c` builds a writable in-memory tree at boot.
- Sample apps are compiled as freestanding ELF64 binaries and embedded into the kernel image.
- `run hello.elf` loads ELF program segments into memory and jumps to the app entry point.

## Graphics layer

- `kernel/graphics.c` provides:
- `draw_pixel(x, y, color)`
- `draw_rect(x, y, width, height, color)`
- `draw_border(x, y, width, height, color)`
- `draw_text(x, y, text, color)`
- All rendering happens in an offscreen backbuffer and is copied to the framebuffer with `graphics_present()`.
- Text is drawn using a built-in 8x8 bitmap font stored as static byte arrays.

## Window manager

- `kernel/wm.c` implements a minimal WM on top of the framebuffer graphics layer.
- The WM uses a small built-in theme for desktop, bar, active/inactive titles, borders, text, and shadows.
- Every window stores position, size, title, active flag, and text content.
- The WM recalculates the full tiling layout after every create/close operation.
- One window fills the workspace, two windows split it 50/50, and 3+ windows use a simple grid.
- The active window is highlighted through title-bar color.
- Windows have gaps, a titlebar, subtle text-mode shadows, and a compact top bar.
- Redraws happen on state changes and once per second for the clock.

### Keyboard controls

- `Alt + Enter` - create a new terminal window
- `Alt + Tab` - switch focus to the next window
- `Alt + Q` - close the active window

### Terminal commands inside a window

- `help`
- `ls`
- `cd`
- `pwd`
- `cat`
- `run <name>.lx`
- `clear`
- `mem`
- `uptime`

## Example shell session

```text
ls
cd /apps
run notes.lx
Alt+Tab
Alt+Enter
```

## Real hardware notes

- The ISO is GRUB-bootable and works in QEMU.
- The current desktop path is based on the GRUB-provided framebuffer and does not need OpenGL or GPU acceleration.
- A good next step is improving framebuffer fallback handling and eventually adding a disk-backed filesystem.
