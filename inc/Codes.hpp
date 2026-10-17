#ifndef CODES_HPP
#define CODES_HPP

#include "../lib/hw.h"

// System call codes
enum SyscallCode {
    MEM_ALLOC = 0x01,
    MEM_FREE = 0x02,
    THREAD_CREATE = 0x11,
    THREAD_EXIT = 0x12,
    THREAD_DISPATCH = 0x13,
    THREAD_GET_ID = 0x14,
    THREAD_JOIN = 0x15,
    SEM_OPEN = 0x21,
    SEM_CLOSE = 0x22,
    SEM_WAIT = 0x23,
    SEM_SIGNAL = 0x24,
    SEM_WAIT_N = 0x25,
    SEM_SIGNAL_N = 0x26,
    TIME_SLEEP = 0x31,
    GETC = 0x41,
    PUTC = 0x42
};

// Causes of entering the trap handler
enum TrapCause : uint64 {
    ECALL_FROM_USER = 0x0000000000000008UL,
    ECALL_FROM_SUPERVISOR = 0x0000000000000009UL,
    TIMER_INTERRUPT = 0x8000000000000001UL,
    EXTERNAL_INTERRUPT = 0x8000000000000009UL
};

// Error codes
enum ErrorCode {
    SUCCESS = 0,
    ERR_NULL_ARG = -1,
    ERR_INVALID_ARG = -2,
    ERR_ALLOC_FAILED = -3,
    ERR_FREE_FAILED = -4,
    ERR_INVALID_HANDLE = -5,
    ERR_SEM_DEALLOCATED = -6,
    ERR_UNKNOWN_SYSCALL = -7
};

#endif