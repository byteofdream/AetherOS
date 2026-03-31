#include "scheduler.h"
#include "heap.h"
#include "string.h"

typedef struct context {
    uint64_t rsp;
    uint64_t rbx;
    uint64_t rbp;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
} context_t;

struct task {
    char name[32];
    context_t context;
    uint8_t *stack;
    bool active;
};

extern void context_switch(context_t *old_ctx, context_t *new_ctx);
extern void task_bootstrap(void);

static task_t tasks[8];
static size_t task_count;
static size_t current_task;
static context_t kernel_context;

void task_boot_entry(task_entry_t entry, void *arg) {
    entry(arg);
    scheduler_exit();
}

void scheduler_init(void) {
    memset(tasks, 0, sizeof(tasks));
    task_count = 0;
    current_task = 0;
}

bool scheduler_create_task(const char *name, task_entry_t entry, void *arg) {
    if (task_count >= ARRAY_SIZE(tasks)) {
        return false;
    }

    task_t *task = &tasks[task_count++];
    strncpy(task->name, name, sizeof(task->name) - 1);
    task->stack = kmalloc(16384);
    if (!task->stack) {
        return false;
    }

    uint64_t *stack_top = (uint64_t *)(task->stack + 16384);
    *(--stack_top) = (uint64_t)arg;
    *(--stack_top) = (uint64_t)entry;
    *(--stack_top) = (uint64_t)task_bootstrap;
    task->context.rsp = (uint64_t)stack_top;
    task->active = true;
    return true;
}

void scheduler_start(void) {
    if (task_count == 0) {
        return;
    }
    current_task = 0;
    context_switch(&kernel_context, &tasks[0].context);
}

void scheduler_yield(void) {
    if (task_count == 0) {
        return;
    }

    size_t start = current_task;
    do {
        current_task = (current_task + 1) % task_count;
        if (tasks[current_task].active) {
            context_switch(&tasks[start].context, &tasks[current_task].context);
            return;
        }
    } while (current_task != start);
}

void scheduler_exit(void) {
    tasks[current_task].active = false;

    for (size_t i = 0; i < task_count; ++i) {
        if (tasks[i].active) {
            size_t old = current_task;
            current_task = i;
            context_switch(&tasks[old].context, &tasks[current_task].context);
        }
    }

    context_switch(&tasks[current_task].context, &kernel_context);
}

const char *scheduler_current_name(void) {
    if (task_count == 0 || !tasks[current_task].active) {
        return "kernel";
    }
    return tasks[current_task].name;
}
