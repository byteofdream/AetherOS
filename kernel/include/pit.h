#ifndef KERNEL_PIT_H
#define KERNEL_PIT_H

#include "common.h"

void pit_init(uint32_t frequency);
uint64_t pit_ticks(void);
void pit_sleep(uint64_t ticks);

#endif
