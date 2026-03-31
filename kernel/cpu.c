#include "cpu.h"
#include "string.h"

static cpu_info_t cpu_info;
static bool cpu_initialized = false;

// CPUID instruction
static void cpuid(uint32_t leaf, uint32_t subleaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
    __asm__ volatile(
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "0"(leaf), "2"(subleaf)
    );
}

void cpu_init(void) {
    memset(&cpu_info, 0, sizeof(cpu_info));

    // Get vendor ID
    uint32_t eax, ebx, ecx, edx;
    cpuid(0, 0, &eax, &ebx, &ecx, &edx);
    
    char vendor[13];
    memcpy(vendor + 0, &ebx, 4);
    memcpy(vendor + 4, &edx, 4);
    memcpy(vendor + 8, &ecx, 4);
    vendor[12] = '\0';
    
    memcpy(cpu_info.vendor, vendor, sizeof(cpu_info.vendor));

    // Get processor brand
    for (uint32_t i = 0x80000002; i <= 0x80000004; i++) {
        cpuid(i, 0, &eax, &ebx, &ecx, &edx);
        int offset = (i - 0x80000002) * 16;
        memcpy(cpu_info.brand + offset + 0, &eax, 4);
        memcpy(cpu_info.brand + offset + 4, &ebx, 4);
        memcpy(cpu_info.brand + offset + 8, &ecx, 4);
        memcpy(cpu_info.brand + offset + 12, &edx, 4);
    }

    // Get features (leaf 1)
    cpuid(1, 0, &eax, &ebx, &ecx, &edx);
    cpu_info.stepping = eax & 0xF;
    cpu_info.model = (eax >> 4) & 0xF;
    cpu_info.family = (eax >> 8) & 0xF;
    cpu_info.type = (eax >> 12) & 0x3;

    cpu_info.features_edx = edx;  // ECX/EDX have feature flags
    cpu_info.features_ecx = ecx;

    // Check for extended features (leaf 7)
    cpuid(7, 0, &eax, &ebx, &ecx, &edx);
    cpu_info.extended_features_ebx = ebx;
    cpu_info.extended_features_ecx = ecx;

    // Get cache/tlb info
    cpuid(0x80000006, 0, &eax, &ebx, &ecx, &edx);
    cpu_info.l2_cache_size = (ecx >> 16) & 0xFFFF;

    cpu_initialized = true;
}

const cpu_info_t* cpu_get_info(void) {
    if (!cpu_initialized) {
        cpu_init();
    }
    return &cpu_info;
}

bool cpu_has_feature(cpu_feature_t feature) {
    if (!cpu_initialized) {
        cpu_init();
    }

    switch (feature) {
        case CPU_FEATURE_FPU:      return (cpu_info.features_edx & (1 << 0)) != 0;
        case CPU_FEATURE_VME:      return (cpu_info.features_edx & (1 << 1)) != 0;
        case CPU_FEATURE_DE:       return (cpu_info.features_edx & (1 << 2)) != 0;
        case CPU_FEATURE_PSE:      return (cpu_info.features_edx & (1 << 3)) != 0;
        case CPU_FEATURE_TSC:      return (cpu_info.features_edx & (1 << 4)) != 0;
        case CPU_FEATURE_MSR:      return (cpu_info.features_edx & (1 << 5)) != 0;
        case CPU_FEATURE_PAE:      return (cpu_info.features_edx & (1 << 6)) != 0;
        case CPU_FEATURE_MCE:      return (cpu_info.features_edx & (1 << 7)) != 0;
        case CPU_FEATURE_CX8:      return (cpu_info.features_edx & (1 << 8)) != 0;
        case CPU_FEATURE_APIC:     return (cpu_info.features_edx & (1 << 9)) != 0;
        case CPU_FEATURE_SEP:      return (cpu_info.features_edx & (1 << 11)) != 0;
        case CPU_FEATURE_MTRR:     return (cpu_info.features_edx & (1 << 12)) != 0;
        case CPU_FEATURE_PGE:      return (cpu_info.features_edx & (1 << 13)) != 0;
        case CPU_FEATURE_MCA:      return (cpu_info.features_edx & (1 << 14)) != 0;
        case CPU_FEATURE_CMOV:     return (cpu_info.features_edx & (1 << 15)) != 0;
        case CPU_FEATURE_PAT:      return (cpu_info.features_edx & (1 << 16)) != 0;
        case CPU_FEATURE_PSE36:    return (cpu_info.features_edx & (1 << 17)) != 0;
        case CPU_FEATURE_PSN:      return (cpu_info.features_edx & (1 << 18)) != 0;
        case CPU_FEATURE_CLFSH:    return (cpu_info.features_edx & (1 << 19)) != 0;
        case CPU_FEATURE_DS:       return (cpu_info.features_edx & (1 << 21)) != 0;
        case CPU_FEATURE_ACPI:     return (cpu_info.features_edx & (1 << 22)) != 0;
        case CPU_FEATURE_MMX:      return (cpu_info.features_edx & (1 << 23)) != 0;
        case CPU_FEATURE_FXSR:     return (cpu_info.features_edx & (1 << 24)) != 0;
        case CPU_FEATURE_SSE:      return (cpu_info.features_edx & (1 << 25)) != 0;
        case CPU_FEATURE_SSE2:     return (cpu_info.features_edx & (1 << 26)) != 0;
        case CPU_FEATURE_SS:       return (cpu_info.features_edx & (1 << 27)) != 0;
        case CPU_FEATURE_HTT:      return (cpu_info.features_edx & (1 << 28)) != 0;
        case CPU_FEATURE_TM:       return (cpu_info.features_edx & (1 << 29)) != 0;
        case CPU_FEATURE_IA64:     return (cpu_info.features_edx & (1 << 30)) != 0;
        case CPU_FEATURE_PBE:      return (cpu_info.features_edx & (1 << 31)) != 0;

        // ECX features (leaf 1)
        case CPU_FEATURE_SSE3:     return (cpu_info.features_ecx & (1 << 0)) != 0;
        case CPU_FEATURE_PCLMUL:   return (cpu_info.features_ecx & (1 << 1)) != 0;
        case CPU_FEATURE_DTES64:   return (cpu_info.features_ecx & (1 << 2)) != 0;
        case CPU_FEATURE_MONITOR:  return (cpu_info.features_ecx & (1 << 3)) != 0;
        case CPU_FEATURE_DS_CPL:   return (cpu_info.features_ecx & (1 << 4)) != 0;
        case CPU_FEATURE_VMX:      return (cpu_info.features_ecx & (1 << 5)) != 0;
        case CPU_FEATURE_SMX:      return (cpu_info.features_ecx & (1 << 6)) != 0;
        case CPU_FEATURE_EST:      return (cpu_info.features_ecx & (1 << 7)) != 0;
        case CPU_FEATURE_TM2:      return (cpu_info.features_ecx & (1 << 8)) != 0;
        case CPU_FEATURE_SSSE3:    return (cpu_info.features_ecx & (1 << 9)) != 0;
        case CPU_FEATURE_CID:      return (cpu_info.features_ecx & (1 << 10)) != 0;
        case CPU_FEATURE_FMA:      return (cpu_info.features_ecx & (1 << 12)) != 0;
        case CPU_FEATURE_CX16:     return (cpu_info.features_ecx & (1 << 13)) != 0;
        case CPU_FEATURE_XTPR:     return (cpu_info.features_ecx & (1 << 14)) != 0;
        case CPU_FEATURE_PDCM:     return (cpu_info.features_ecx & (1 << 15)) != 0;
        case CPU_FEATURE_PCID:     return (cpu_info.features_ecx & (1 << 17)) != 0;
        case CPU_FEATURE_DCA:      return (cpu_info.features_ecx & (1 << 18)) != 0;
        case CPU_FEATURE_SSE41:    return (cpu_info.features_ecx & (1 << 19)) != 0;
        case CPU_FEATURE_SSE42:    return (cpu_info.features_ecx & (1 << 20)) != 0;
        case CPU_FEATURE_X2APIC:   return (cpu_info.features_ecx & (1 << 21)) != 0;
        case CPU_FEATURE_MOVBE:    return (cpu_info.features_ecx & (1 << 22)) != 0;
        case CPU_FEATURE_POPCNT:   return (cpu_info.features_ecx & (1 << 23)) != 0;
        case CPU_FEATURE_TSC_DEADLINE: return (cpu_info.features_ecx & (1 << 24)) != 0;
        case CPU_FEATURE_AES:      return (cpu_info.features_ecx & (1 << 25)) != 0;
        case CPU_FEATURE_XSAVE:    return (cpu_info.features_ecx & (1 << 26)) != 0;
        case CPU_FEATURE_OSXSAVE:  return (cpu_info.features_ecx & (1 << 27)) != 0;
        case CPU_FEATURE_AVX:      return (cpu_info.features_ecx & (1 << 28)) != 0;
        case CPU_FEATURE_F16C:     return (cpu_info.features_ecx & (1 << 29)) != 0;
        case CPU_FEATURE_RDRND:    return (cpu_info.features_ecx & (1 << 30)) != 0;

        // Extended features (leaf 7)
        case CPU_FEATURE_AVX2:     return (cpu_info.extended_features_ebx & (1 << 5)) != 0;
        case CPU_FEATURE_BMI1:     return (cpu_info.extended_features_ebx & (1 << 3)) != 0;
        case CPU_FEATURE_BMI2:     return (cpu_info.extended_features_ebx & (1 << 8)) != 0;
        case CPU_FEATURE_ADX:      return (cpu_info.extended_features_ebx & (1 << 19)) != 0;
        case CPU_FEATURE_SHA:      return (cpu_info.extended_features_ebx & (1 << 29)) != 0;

        default:
            return false;
    }
}
