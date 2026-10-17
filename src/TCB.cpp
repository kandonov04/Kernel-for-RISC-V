#include "../inc/TCB.hpp"
#include "../inc/Scheduler.hpp"
#include "../inc/MemoryAllocator.hpp"
#include "../inc/Pool.hpp"
#include "../inc/Riscv.hpp"
#include "../inc/syscall_c.h"
#include "../inc/Codes.hpp"
#include "../inc/SleepingThreadList.hpp"
#include "../inc/MySemaphore.hpp"

TCB* TCB::running = nullptr;
uint64 TCB::timeSliceCounter = 0;
uint64 TCB::liveThreadCount = 0;
Queue<TCB> TCB::finishedQueue;
TCB* TCB::idleThread = nullptr;
uint64 TCB::nextId = 1;

void* TCB::operator new(size_t) noexcept {
    return Pool<TCB>::allocate();
}

void TCB::operator delete(void* address) noexcept {
    Pool<TCB>::deallocate(address);
}

TCB::TCB(Body body, void* arg, uint64* stack, uint64 timeSlice, bool kernelThread) :
        body(body),
        arg(arg),
        stackTop(stack),
        // Execution starts from the threadWrapper function,
        // except for internal kernel threads, which start directly from their body and therefore remain in system mode, with interrupts masked
        context({kernelThread ? (uint64)body : (uint64)&threadWrapper, stack != nullptr ? (uint64)stack - 96 : 0}),
        timeSlice(timeSlice),
        state(READY),
        units(0),
        blockResult(SUCCESS),
        sleepTime(0),
        id(0),
        userHandle(nullptr),
        joinSem(nullptr),
        joinCount(0),
        next(nullptr) {
    if (body != nullptr) {
        Scheduler::put(this);
    }
}

TCB::~TCB() {
    if (stackTop != nullptr) {
        void* base = (void*)((uint64)stackTop - DEFAULT_STACK_SIZE);
        MemoryAllocator::getInstance().memFree(base);
    }
}

TCB* TCB::createThread(Body body, void* arg, uint64* stack) {
    return new TCB(body, arg, stack, DEFAULT_TIME_SLICE, false);
}

TCB* TCB::createKernelThread(Body body, uint64* stack) {
    return new TCB(body, nullptr, stack, DEFAULT_TIME_SLICE, true);
}

// The idle thread's body simply yields the processor immediately, and it is always present in the ready queue
void TCB::idleBody(void *) {
    while (true) {
        thread_dispatch();
    }
}

void TCB::initialize() {
    // The kernel allocates the idle thread's stack because it does not go through the C API layer
    void* space = MemoryAllocator::getInstance().memAlloc(MemoryAllocator::bytesToBlocks(DEFAULT_STACK_SIZE));
    if (space == nullptr) {
        return;
    }
    uint64* stackTop = (uint64*)((uint64)space + DEFAULT_STACK_SIZE);
    idleThread = createThread(idleBody, nullptr, stackTop);
}

// SYNCHRONOUS CONTEXT SWITCHING
// The thread that calls dispatch is suspended inside contextSwitch, on its own stack, and resumes there when the scheduler assigns the processor to it again
void TCB::dispatch() {
    // Release terminated threads
    collectFinished();

    TCB* old = running;

    switch (old -> state) {
        case FINISHED:
            // The terminated thread's stack must not be released here because execution is currently taking place on it
            finishedQueue.addLast(old);
            break;
        case BLOCKED:
        case SLEEPING:
            break;
        default:
            old -> state = READY;
            Scheduler::put(old);
            break;
    }

    running = Scheduler::get();
    running -> state = RUNNING;
    // Every thread that gets the processor receives its full time slice
    timeSliceCounter = 0;
    contextSwitch(&old -> context, &running -> context);
}

// The first function executed by every newly created thread!
void TCB::threadWrapper() {
    Riscv::popSppSpie();
    running -> body(running -> arg);
    thread_exit();
}

void TCB::join(TCB** handle) {
    if (handle == nullptr) {
        return;
    }
    TCB* thread = *handle;
    if (thread == nullptr || thread == running) {
        return;
    }

    thread -> joinCount++;

    // The first thread that waits creates the semaphore, and the remaining threads use the same one
    if (thread -> joinSem == nullptr) {
        thread -> joinSem = MySemaphore::createSemaphore(0);
        if (thread -> joinSem == nullptr) {
            thread -> joinCount--;
            return;
        }
    }

    thread -> joinSem -> wait(1);

    thread -> joinCount--;
}

// Terminates the current thread and yields the processor to the next ready thread
void TCB::exitCurrent() {
    running -> state = FINISHED;

    // Closing the semaphore immediately releases all threads waiting for this thread to terminate
    if (running -> joinSem != nullptr) {
        running -> joinSem -> close();
    }

    if (running -> userHandle != nullptr) {
        *running -> userHandle = nullptr;
    }

    if (liveThreadCount > 0) {
        liveThreadCount--;
    }
    dispatch();
}

// Handles one timer period
void TCB::timerTick() {
    SleepingThreadList::tick();
    if (running == nullptr) {
        return;
    }
    timeSliceCounter++;
    if (timeSliceCounter >= running -> timeSlice) {
        dispatch();
    }
}

// Releases the resources of terminated threads
// A thread that is still being waited on by another thread through join is placed at the end of the queue again and is not released
void TCB::collectFinished() {
    uint64 count = finishedQueue.size();
    for (uint64 i = 0; i < count; i++) {
        TCB* tcb = finishedQueue.removeFirst();
        if (tcb -> joinCount == 0) {
            delete tcb;
        } else {
            finishedQueue.addLast(tcb);
        }
    }
}