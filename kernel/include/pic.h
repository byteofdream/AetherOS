#ifndef KERNEL_PIC_H
#define KERNEL_PIC_H

#include "common.h"

void pic_remap(uint8_t offset1, uint8_t offset2);
void pic_send_eoi(uint8_t irq);
void pic_mask_irq(uint8_t irq);
void pic_unmask_irq(uint8_t irq);

#endif
