#ifndef KERNEL_CPU_H
#define KERNEL_CPU_H

#include "common.h"

typedef enum {
    // EDX features (leaf 1)
    CPU_FEATURE_FPU = 0,
    CPU_FEATURE_VME,
    CPU_FEATURE_DE,
    CPU_FEATURE_PSE,
    CPU_FEATURE_TSC,
    CPU_FEATURE_MSR,
    CPU_FEATURE_PAE,
    CPU_FEATURE_MCE,
    CPU_FEATURE_CX8,
    CPU_FEATURE_APIC,
    CPU_FEATURE_SEP,
    CPU_FEATURE_MTRR,
    CPU_FEATURE_PGE,
    CPU_FEATURE_MCA,
    CPU_FEATURE_CMOV,
    CPU_FEATURE_PAT,
    CPU_FEATURE_PSE36,
    CPU_FEATURE_PSN,
    CPU_FEATURE_CLFSH,
    CPU_FEATURE_DS,
    CPU_FEATURE_ACPI,
    CPU_FEATURE_MMX,
    CPU_FEATURE_FXSR,
    CPU_FEATURE_SSE,
    CPU_FEATURE_SSE2,
    CPU_FEATURE_SS,
    CPU_FEATURE_HTT,
    CPU_FEATURE_TM,
    CPU_FEATURE_IA64,
    CPU_FEATURE_PBE,

    // ECX features (leaf 1)
    CPU_FEATURE_SSE3,
    CPU_FEATURE_PCLMUL,
    CPU_FEATURE_DTES64,
    CPU_FEATURE_MONITOR,
    CPU_FEATURE_DS_CPL,
    CPU_FEATURE_VMX,
    CPU_FEATURE_SMX,
    CPU_FEATURE_EST,
    CPU_FEATURE_TM2,
    CPU_FEATURE_SSSE3,
    CPU_FEATURE_CID,
    CPU_FEATURE_FMA,
    CPU_FEATURE_CX16,
    CPU_FEATURE_XTPR,
    CPU_FEATURE_PDCM,
    CPU_FEATURE_PCID,
    CPU_FEATURE_DCA,
    CPU_FEATURE_SSE41,
    CPU_FEATURE_SSE42,
    CPU_FEATURE_X2APIC,
    CPU_FEATURE_MOVBE,
    CPU_FEATURE_POPCNT,
    CPU_FEATURE_TSC_DEADLINE,
    CPU_FEATURE_AES,
    CPU_FEATURE_XSAVE,
    CPU_FEATURE_OSXSAVE,
    CPU_FEATURE_AVX,
    CPU_FEATURE_F16C,
    CPU_FEATURE_RDRND,

    // Extended features (leaf 7, EBX)
    CPU_FEATURE_BMI1,
    CPU_FEATURE_AVX2,
    CPU_FEATURE_BMI2,
    CPU_FEATURE_ADX,
    CPU_FEATURE_SHA,
} cpu_feature_t;

typedef struct {
    char vendor[13];
    char brand[49];
    uint32_t stepping;
    uint32_t model;
    uint32_t family;
    uint32_t type;
    uint32_t features_edx;
    uint32_t features_ecx;
    uint32_t extended_features_ebx;
    uint32_t extended_features_ecx;
    uint32_t l2_cache_size;
} cpu_info_t;

void cpu_init(void);
const cpu_info_t* cpu_get_info(void);
bool cpu_has_feature(cpu_feature_t feature);

#endif
