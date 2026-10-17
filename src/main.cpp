#include "../lib/hw.h"
#include "../inc/Riscv.hpp"
#include "../inc/TCB.hpp"
#include "../inc/MyConsole.hpp"
#include "../inc/Codes.hpp"
#include "../inc/syscall_c.h"

// Main program of the user application

extern void userMain();

static void userMainWrapper(void*) {
    userMain();
}

// Stopping the emulator
/*"Writing the 32-bit value 0x5555 to address 0x100000 stops the RISC-V processor emulator."*/
static void haltEmulator() {
    *(uint32*)0x100000 = 0x5555;
}

int main() {
    // Initialize the interrupt vector
    Riscv::w_stvec((uint64)&Riscv::supervisorTrap);

    // Thread representing main
    TCB* mainThread = TCB::createThread(nullptr, nullptr, nullptr);
    if (mainThread == nullptr) {
        haltEmulator();
        return 1;
    }
    TCB::running = mainThread;

    // The idle thread must be created before any thread sleeps or gets blocked
    TCB::initialize();

    // Initialize the console - this must be done before enabling interrupts, because console interrupt handling immediately uses the input buffer
    MyConsole::initialize();

    // Now we enable interrupts
    Riscv::ms_sstatus(Riscv::SSTATUS_SIE);

    // Start the user thread
    thread_t userMainThread;
    if (thread_create(&userMainThread, userMainWrapper, nullptr) != SUCCESS) {
        haltEmulator();
        return 1;
    }

    // Main waits for all user application threads to finish, then stops the emulator
    while (TCB::getLiveThreadCount() > 0) {
        thread_dispatch();
    }

    // Normal program termination
    Riscv::mc_sstatus(Riscv::SSTATUS_SIE);
    haltEmulator();
    return 0;
}