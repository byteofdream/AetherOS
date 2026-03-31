// C++ Runtime Support for AetherOS
// Minimal implementation for basic C++ features

#include "../include/app_api.h"
#include <stddef.h>

extern "C" {
    void* __cxa_allocate_exception(size_t thrown_size);
    void __cxa_free_exception(void *thrown_exception);
    void __cxa_throw(void* thrown_exception, void* tinfo, void (*dest)(void*));
    void* __cxa_begin_catch(void* exceptionObject);
    void __cxa_end_catch();
}

app_syscalls_t *g_syscalls;

void* operator new(size_t size) {
    // Simple allocation (would need actual memory management)
    extern void* malloc(size_t);
    return malloc(size);
}

void* operator new[](size_t size) {
    extern void* malloc(size_t);
    return malloc(size);
}

void operator delete(void* ptr) {
    extern void free(void*);
    free(ptr);
}

void operator delete[](void* ptr) {
    extern void free(void*);
    free(ptr);
}

void* __cxa_allocate_exception(size_t thrown_size) {
    extern void* malloc(size_t);
    return malloc(thrown_size);
}

void __cxa_free_exception(void *thrown_exception) {
    extern void free(void*);
    free(thrown_exception);
}

void __cxa_throw(void* thrown_exception, void* tinfo, void (*dest)(void*)) {
    // Simplified - just abort
    if (g_syscalls) {
        g_syscalls->write("Exception thrown\n");
    }
    while(1) asm("hlt");
}

void* __cxa_begin_catch(void* exceptionObject) {
    return exceptionObject;
}

void __cxa_end_catch() {
}

// Pure virtual function handling
extern "C" void __cxa_pure_virtual() {
    if (g_syscalls) {
        g_syscalls->write("Pure virtual function called\n");
    }
}

// RTTI
extern "C" {
    class __class_type_info {
    public:
        virtual ~__class_type_info() {}
        virtual void* cast_to(void* obj) { return nullptr; }
    };

    class __si_class_type_info : public __class_type_info {
    public:
        virtual ~__si_class_type_info() {}
    };
}

// Guard variables for static initialization
extern "C" {
    typedef int guard_t;
    int __cxa_guard_acquire(guard_t *guard);
    void __cxa_guard_release(guard_t *guard);
    void __cxa_guard_abort(guard_t *guard);

    int __cxa_guard_acquire(guard_t *guard) {
        return !(*guard);
    }

    void __cxa_guard_release(guard_t *guard) {
        *guard = 1;
    }

    void __cxa_guard_abort(guard_t *guard) {
        (void)guard;
    }
}
