#include "../inc/MyConsole.hpp"
#include "../inc/MemoryAllocator.hpp"
#include "../inc/TCB.hpp"

// Access to the serial communication controller registers
static volatile char* statusReg() {
    return (volatile char*)CONSOLE_STATUS;
}

static volatile char* txReg() {
    return (volatile char*)CONSOLE_TX_DATA;
}

static volatile char* rxReg() {
    return (volatile char*)CONSOLE_RX_DATA;
}

MyConsole& MyConsole::getInstance() {
    static MyConsole instance;
    return instance;
}

void MyConsole::initialize() {
    MyConsole& console = getInstance();

    if (!console.outputBuffer.initialize(OUTPUT_CAPACITY)) {
        return;
    }
    if (!console.inputBuffer.initialize(INPUT_CAPACITY)) {
        return;
    }

    // The kernel allocates the internal thread's stack because it does not go through the C API layer
    void* space = MemoryAllocator::getInstance().memAlloc(MemoryAllocator::bytesToBlocks(DEFAULT_STACK_SIZE));
    if (space == nullptr) {
        return;
    }
    uint64* stackTop = (uint64*)((uint64)space + DEFAULT_STACK_SIZE);

    TCB::createKernelThread(outputThreadBody, stackTop);
}


// Operations called from kernel code

void MyConsole::putChar(char chr) {
    outputBuffer.put(chr);
}

char MyConsole::getChar() {
    return inputBuffer.get();
}

char MyConsole::getOutputChar() {
    return outputBuffer.get();
}

void MyConsole::handleInterrupt() {
    /*"When handling an interrupt caused by a character received from the keyboard, input characters must be placed in the input buffer."*/
    while ((*statusReg() & CONSOLE_RX_STATUS_BIT) != 0) {
        char chr = *rxReg();
        inputBuffer.putFromInterrupt(chr);
    }
}

// Internal kernel thread - consumer of the output buffer
// This thread remains in the processor's system mode, with interrupts masked (it executes as kernel code)
// Therefore, it calls buffer operations directly, without a system call
// This function must never return because there is no one who can terminate the thread
void MyConsole::outputThreadBody(void *) {
    MyConsole& console = getInstance();

    while (true) {
        // Blocks the thread until a character appears in the output buffer
        char chr = console.getOutputChar();

        /*"When the controller is ready to transmit data, it generates an interrupt.
        Data can then be transferred to the controller as long as the ready bit is set."*/
        while ((*statusReg() & CONSOLE_TX_STATUS_BIT) == 0) {}

        *txReg() = chr;
    }
}