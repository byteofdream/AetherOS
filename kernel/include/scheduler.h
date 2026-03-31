#ifndef KERNEL_SCHEDULER_H
#define KERNEL_SCHEDULER_H

#include "common.h"

typedef struct task task_t;
typedef void (*task_entry_t)(void *arg);

void scheduler_init(void);
bool scheduler_create_task(const char *name, task_entry_t entry, void *arg);
void scheduler_start(void);
void scheduler_yield(void);
void scheduler_exit(void);
const char *scheduler_current_name(void);

#endif
