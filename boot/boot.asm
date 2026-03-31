[BITS 32]

section .multiboot
align 8
mb2_header:
    dd 0xe85250d6
    dd 0
    dd mb2_header_end - mb2_header
    dd -(0xe85250d6 + 0 + (mb2_header_end - mb2_header))

    dw 5
    dw 0
    dd 20
    dd 1024
    dd 768
    dd 32

    align 8
    dw 0
    dw 0
    dd 8
mb2_header_end:

section .boot.data
align 16
gdt64:
    dq 0
    dq 0x00af9a000000ffff
    dq 0x00af92000000ffff
gdt64_ptr:
    dw gdt64_end - gdt64 - 1
    dd gdt64
gdt64_end:

section .bss
align 16
boot_stack:
    resb 16384
boot_stack_top:

align 4096
pml4_table:
    resq 512
pdpt_table:
    resq 512
pd_table0:
    resq 512
pd_table1:
    resq 512
pd_table2:
    resq 512
pd_table3:
    resq 512

section .boot.text
global _start
extern kernel_entry64

_start:
    cli
    mov esp, boot_stack_top
    mov [mb_magic], eax
    mov [mb_info], ebx

    lgdt [gdt64_ptr]

    mov edi, pml4_table
    xor eax, eax
    mov ecx, 512 * 12
    rep stosd

    mov eax, pdpt_table
    or eax, 0x3
    mov [pml4_table], eax

    mov eax, pd_table0
    or eax, 0x3
    mov [pdpt_table], eax

    mov eax, pd_table1
    or eax, 0x3
    mov [pdpt_table + 8], eax

    mov eax, pd_table2
    or eax, 0x3
    mov [pdpt_table + 16], eax

    mov eax, pd_table3
    or eax, 0x3
    mov [pdpt_table + 24], eax

    xor ecx, ecx
.map_loop:
    mov eax, ecx
    shl eax, 21
    or eax, 0x83
    mov [pd_table0 + ecx * 8], eax
    mov dword [pd_table0 + ecx * 8 + 4], 0
    add eax, 0x40000000
    mov [pd_table1 + ecx * 8], eax
    mov dword [pd_table1 + ecx * 8 + 4], 0
    add eax, 0x40000000
    mov [pd_table2 + ecx * 8], eax
    mov dword [pd_table2 + ecx * 8 + 4], 0
    add eax, 0x40000000
    mov [pd_table3 + ecx * 8], eax
    mov dword [pd_table3 + ecx * 8 + 4], 0
    inc ecx
    cmp ecx, 512
    jne .map_loop

    mov eax, pml4_table
    mov cr3, eax

    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    mov ecx, 0xc0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    jmp 0x08:long_mode_start

[BITS 64]
long_mode_start:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov rsp, boot_stack_top

    mov edi, [mb_magic]
    mov esi, [mb_info]
    mov rdi, rsi
    call kernel_entry64

.hang:
    hlt
    jmp .hang

section .boot.data
mb_magic: dd 0
mb_info: dd 0
