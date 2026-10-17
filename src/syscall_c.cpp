#include "../inc/syscall_c.h"
#include "../inc/Codes.hpp"
#include "../inc/MemoryAllocator.hpp"

// Common part of all system calls
// ABI convention:
// * a0 - system call code, also used as the return value
// * a1..a7 - arguments, in left-to-right order according to the C API function signature
static uint64 syscall(uint64 code, uint64 arg1 = 0, uint64 arg2 = 0, uint64 arg3 = 0, uint64 arg4 = 0, uint64 arg5 = 0, uint64 arg6 = 0, uint64 arg7 = 0) {
    // Forces the compiler to place the value in the specified register and not use it as a temporary
    register uint64 a0 __asm__("a0") = code;
    register uint64 a1 __asm__("a1") = arg1;
    register uint64 a2 __asm__("a2") = arg2;
    register uint64 a3 __asm__("a3") = arg3;
    register uint64 a4 __asm__("a4") = arg4;
    register uint64 a5 __asm__("a5") = arg5;
    register uint64 a6 __asm__("a6") = arg6;
    register uint64 a7 __asm__("a7") = arg7;
    __asm__ volatile ("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6), "r"(a7) : "memory");
    return a0;
}

void* mem_alloc(size_t size) {
    if (size == 0) {
        return nullptr;
    }
    size_t blocks = MemoryAllocator::bytesToBlocks(size);
    return (void*)syscall(MEM_ALLOC, blocks);
}

int mem_free(void* address) {
    if (address == nullptr) {
        return ERR_NULL_ARG;
    }
    return (int)syscall(MEM_FREE, (uint64)address);
}

int thread_create(thread_t* handle, void(*start_routine)(void*), void* arg) {
    if (handle == nullptr) {
        return ERR_NULL_ARG;
    }
    // The stack is allocated here, in the C API layer
    void* stackSpace = nullptr;
    if (start_routine) {
        void* stack = mem_alloc(DEFAULT_STACK_SIZE);
        if (stack == nullptr) {
            return ERR_ALLOC_FAILED;
        }
        stackSpace = (void*)((uint64)stack + DEFAULT_STACK_SIZE);
    }
    int ret = (int)syscall(THREAD_CREATE, (uint64)handle, (uint64)start_routine, (uint64)arg, (uint64)stackSpace);

    // If the kernel failed to create the thread, the stack must be deallocated
    if (ret != SUCCESS && stackSpace) {
        mem_free((void*)((uint64)stackSpace - DEFAULT_STACK_SIZE));
    }
    return ret;
}

int thread_exit() {
    return (int)syscall(THREAD_EXIT);
}

void thread_dispatch() {
    syscall(THREAD_DISPATCH);
}

int thread_get_id() {
    return (int)syscall(THREAD_GET_ID);
}

void thread_join(thread_t* handle) {
    if (handle == nullptr) {
        return;
    }
    syscall(THREAD_JOIN, (uint64)handle);
}

int sem_open(sem_t* handle, unsigned init) {
    if (handle == nullptr) {
        return ERR_NULL_ARG;
    }
    return (int)syscall(SEM_OPEN, (uint64)handle, init);
}

int sem_close(sem_t handle) {
    if (handle == nullptr) {
        return ERR_INVALID_HANDLE;
    }
    return (int)syscall(SEM_CLOSE, (uint64)handle);
}

int sem_wait(sem_t id) {
    if (id == nullptr) {
        return ERR_INVALID_HANDLE;
    }
    return (int)syscall(SEM_WAIT, (uint64)id);
}

int sem_signal(sem_t id) {
    if (id == nullptr) {
        return ERR_INVALID_HANDLE;
    }
    return (int)syscall(SEM_SIGNAL, (uint64)id);
}

int sem_wait_n(sem_t id, unsigned n) {
    if (id == nullptr) {
        return ERR_INVALID_HANDLE;
    }
    return (int)syscall(SEM_WAIT_N, (uint64)id, n);
}

int sem_signal_n(sem_t id, unsigned n) {
    if (id == nullptr) {
        return ERR_INVALID_HANDLE;
    }
    return (int)syscall(SEM_SIGNAL_N, (uint64)id, n);
}

int time_sleep(time_t periods) {
    return (int)syscall(TIME_SLEEP, periods);
}

char getc() {
    return (char)syscall(GETC);
}

void putc(char chr) {
    syscall(PUTC, (uint64)chr);
}