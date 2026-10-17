#include "../inc/Riscv.hpp"
#include "../inc/Codes.hpp"
#include "../inc/MemoryAllocator.hpp"
#include "../inc/Print.hpp"
#include "../inc/TCB.hpp"
#include "../inc/MySemaphore.hpp"
#include "../inc/SleepingThreadList.hpp"
#include "../inc/MyConsole.hpp"

void Riscv::popSppSpie() {
    __asm__ volatile ("csrc sstatus, %0" : : "r"(SSTATUS_SPP));
    __asm__ volatile ("csrw sepc, ra");
    __asm__ volatile ("sret");
}

void Riscv::handleSupervisorTrap() {
    uint64 scause = r_scause();
    // sepc and sstatus are saved in local variables because a context switch may occur during handling, so upon return they may belong to another thread
    volatile uint64 sepc = r_sepc();
    volatile uint64 sstatus = r_sstatus();

    if (scause == ECALL_FROM_USER || scause == ECALL_FROM_SUPERVISOR) {
        // interrupt: no, cause code: environment call from S-mode (9) or U-mode (8)

        // sepc points to the ecall instruction itself, so we skip it
        sepc += 4;

        uint64 a0;
        __asm__ volatile("ld %0, 10 * 8(fp)" : "=r"(a0));

        switch (a0) {
            case MEM_ALLOC: {
                uint64 a1;
                __asm__ volatile("ld %0, 11 * 8(fp)" : "=r"(a1));
                void* ptr = MemoryAllocator::getInstance().memAlloc(a1);
                __asm__ volatile ("sd %[ptr], 10 * 8(fp)" : : [ptr] "r"(ptr) : "memory");
                break;
            }
            case MEM_FREE: {
                uint64 a1;
                __asm__ volatile("ld %0, 11 * 8(fp)" : "=r"(a1));
                uint64 ret = (uint64)MemoryAllocator::getInstance().memFree((void*)a1);
                __asm__ volatile ("sd %[ret], 10 * 8(fp)" : : [ret] "r"(ret) : "memory");
                break;
            }
            case THREAD_CREATE: {
                uint64 a1, a2, a3, a4;
                __asm__ volatile("ld %0, 11 * 8(fp)" : "=r"(a1));
                __asm__ volatile("ld %0, 12 * 8(fp)" : "=r"(a2));
                __asm__ volatile("ld %0, 13 * 8(fp)" : "=r"(a3));
                __asm__ volatile("ld %0, 14 * 8(fp)" : "=r"(a4));
                TCB** handle = (TCB**)a1;
                TCB* tcb = TCB::createThread((TCB::Body)a2, (void*)a3, (uint64*)a4);
                if (handle) {
                    *handle = tcb;
                }
                if (tcb) {
                    // The Main and Idle threads do not pass through here
                    TCB::liveThreadCount++;
                    TCB::setId(tcb);
                    tcb -> userHandle = handle;
                }
                uint64 ret = (uint64)(tcb != nullptr ? SUCCESS : ERR_ALLOC_FAILED);
                __asm__ volatile ("sd %[ret], 10 * 8(fp)" : : [ret] "r"(ret) : "memory");
                break;
            }
            case THREAD_EXIT: {
                TCB::exitCurrent();
                break;
            }
            case THREAD_DISPATCH: {
                TCB::dispatch();
                break;
            }
            case THREAD_GET_ID: {
                uint64 ret = TCB::getId();
                __asm__ volatile ("sd %[ret], 10 * 8(fp)" : : [ret] "r"(ret) : "memory");
                break;
            }
            case THREAD_JOIN: {
                uint64 a1;
                __asm__ volatile("ld %0, 11 * 8(fp)" : "=r"(a1));
                TCB::join((TCB**)a1);
                break;
            }
            case SEM_OPEN: {
                uint64 a1, a2;
                __asm__ volatile("ld %0, 11 * 8(fp)" : "=r"(a1));
                __asm__ volatile("ld %0, 12 * 8(fp)" : "=r"(a2));

                MySemaphore** handle = (MySemaphore**)a1;
                MySemaphore* sem = MySemaphore::createSemaphore(a2);
                if (handle) {
                    *handle = sem;
                }

                uint64 ret = (uint64)(sem != nullptr ? SUCCESS : ERR_ALLOC_FAILED);
                __asm__ volatile ("sd %[ret], 10 * 8(fp)" : : [ret] "r"(ret) : "memory");
                break;
            }
            case SEM_CLOSE: {
                uint64 a1;
                __asm__ volatile("ld %0, 11 * 8(fp)" : "=r"(a1));

                MySemaphore* sem = (MySemaphore*)a1;
                uint64 ret;
                if (sem == nullptr) {
                    ret = (uint64)ERR_INVALID_HANDLE;
                } else {
                    ret = (uint64)sem -> close();
                    // Unblocked threads no longer access the semaphore object, so it can be destroyed immediately
                    delete sem;
                }
                __asm__ volatile ("sd %[ret], 10 * 8(fp)" : : [ret] "r"(ret) : "memory");
                break;
            }
            case SEM_WAIT:
            case SEM_WAIT_N: {
                uint64 a1, a2 = 1;
                __asm__ volatile("ld %0, 11 * 8(fp)" : "=r"(a1));
                if (a0 == SEM_WAIT_N) {
                    __asm__ volatile("ld %0, 12 * 8(fp)" : "=r"(a2));
                }

                MySemaphore* sem = (MySemaphore*)a1;
                uint64 ret = sem == nullptr ? (uint64)ERR_INVALID_HANDLE : (uint64)sem -> wait(a2);

                __asm__ volatile ("sd %[ret], 10 * 8(fp)" : : [ret] "r"(ret) : "memory");
                break;
            }
            case SEM_SIGNAL:
            case SEM_SIGNAL_N: {
                uint64 a1, a2 = 1;
                __asm__ volatile("ld %0, 11 * 8(fp)" : "=r"(a1));
                if (a0 == SEM_SIGNAL_N) {
                    __asm__ volatile("ld %0, 12 * 8(fp)" : "=r"(a2));
                }

                MySemaphore* sem = (MySemaphore*)a1;
                uint64 ret = sem == nullptr ? (uint64)ERR_INVALID_HANDLE : (uint64)sem -> signal(a2);

                __asm__ volatile ("sd %[ret], 10 * 8(fp)" : : [ret] "r"(ret) : "memory");
                break;
            }
            case TIME_SLEEP: {
                uint64 a1;
                __asm__ volatile("ld %0, 11 * 8(fp)" : "=r"(a1));

                SleepingThreadList::sleep(a1);

                uint64 ret = SUCCESS;
                __asm__ volatile ("sd %[ret], 10 * 8(fp)" : : [ret] "r"(ret) : "memory");
                break;
            }
            case GETC: {
                uint64 ret = (uint64)MyConsole::getInstance().getChar();
                __asm__ volatile ("sd %[ret], 10 * 8(fp)" : : [ret] "r"(ret) : "memory");
                break;
            }
            case PUTC: {
                uint64 a1;
                __asm__ volatile("ld %0, 11 * 8(fp)" : "=r"(a1));

                MyConsole::getInstance().putChar((char)a1);
                break;
            }
            default: {
                uint64 ret = (uint64)ERR_UNKNOWN_SYSCALL;
                __asm__ volatile ("sd %[ret], 10 * 8(fp)" : : [ret] "r"(ret) : "memory");
                break;
            }
        }
    } else if (scause == TIMER_INTERRUPT) {
        // interrupt: yes, cause code: supervisor software interrupt (timer)

        // Software interrupt, so the request must be cleared manually; otherwise, it would immediately be triggered again
        mc_sip(SIP_SSIP);

        TCB::timerTick();
    } else if (scause == EXTERNAL_INTERRUPT) {
        // interrupt: yes, cause code: supervisor external interrupt (console)

        int irq = plic_claim();
        if (irq == (int)CONSOLE_IRQ) {
            MyConsole::getInstance().handleInterrupt();
        }
        if (irq != 0) {
            plic_complete(irq);
        }
    } else {
        // unexpected trap cause

        myPrintString("\nUnexpected Trap Cause\n");
        myPrintString("scause = ");
        myPrintNumber(scause);
        myPrintString("\nsepc   = ");
        myPrintNumber(r_sepc());
        myPrintString("\nstval  = ");
        myPrintNumber(r_stval());
        myPrintString("\n");
        while (true) {}
    }
    w_sstatus(sstatus);
    w_sepc(sepc);
}