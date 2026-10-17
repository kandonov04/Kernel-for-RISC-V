#ifndef TCB_HPP
#define TCB_HPP

#include "../lib/hw.h"
#include "Scheduler.hpp"

class MySemaphore;

// Thread Control Block - abstraction of a thread in the kernel
// The thread context consists only of the ra and sp registers - everything else is saved on the thread's own stack in the interrupt handler
class TCB {
public:
    // Thread states
    enum State {
        RUNNING, // currently running thread
        READY, // waiting in the ready queue
        BLOCKED, // waiting in a semaphore queue
        SLEEPING, // waiting for the timeout in the list of sleeping threads
        FINISHED // terminated, waiting for deallocation
    };

    ~TCB();

    void* operator new(size_t size) noexcept;
    void operator delete(void* address) noexcept;

    using Body = void(*)(void*);

    // Creates the idle thread, called once during initialization
    /*"Attention should also be paid to how to handle the situation in which there are no user threads among the ready threads.
    A simple solution is to provide an idle thread that does nothing and gets processor time only when there are no other ready threads."*/
    static void initialize();

    // Creates a thread with the specified body and adds it to the ready queue
    static TCB* createThread(Body body, void* arg, uint64* stack);

    // Creates an INTERNAL KERNEL THREAD!
    // Its initial context points directly to the body =>
    // * it executes in kernel mode and may directly call kernel operations
    // * the argument is not passed to the body because there is no wrapper to set it
    // * it must never return from the body because there is no one to call thread_exit
    static TCB* createKernelThread(Body body, uint64* stack);

    State getState() const { return state; }

    void setState(State value) { state = value; }

    bool isFinished() const { return state == FINISHED; }

    uint64 getTimeSlice() const { return timeSlice; }

    // Number of user threads that have not yet terminated
    static uint64 getLiveThreadCount() { return liveThreadCount; }

    // Stops the running thread until the thread pointed to by handle terminates
    static void join(TCB** handle);

    // Terminates the current thread
    static void exitCurrent();

    // Assigns a unique identifier to a user thread
    static void setId(TCB* thread) { thread -> id = nextId++; }

    // Retrieves the unique identifier of the current thread
    static uint64 getId() { return running -> id; }

    // Handles one timer period
    static void timerTick();

    static TCB* running;

private:
    TCB(Body body, void* arg, uint64* stack, uint64 timeSlice, bool kernelThread);

    // Delete the copy constructor and assignment operator
    TCB(const TCB &) = delete;
    TCB &operator=(const TCB &) = delete;

    struct Context {
        uint64 ra;
        uint64 sp;
    };

    Body body;
    void* arg;

    // Points to the end of the separate stack memory region
    uint64* stackTop;
    Context context;
    uint64 timeSlice;
    State state;

    // Number of units the thread is waiting for on a semaphore
    uint64 units;

    // Result that the blocking system call will return when the thread wakes up (SUCCESS or ERR_SEM_DEALLOCATED)
    int blockResult;

    // Remaining sleep time, relative to the previous thread in the list
    time_t sleepTime;

    // Unique identifier of a user thread, starting from 1
    // Kernel threads (main, idle, console) do not have one; their identifier is 0
    uint64 id;

    // The next identifier to be assigned
    static uint64 nextId;

    // Address of the user's handle, so it can be set to null upon termination
    TCB** userHandle;

    // Semaphore on which all threads waiting for this thread to terminate are blocked
    // It is created by the first waiting thread and closed upon termination, thereby unblocking all waiting threads at once
    MySemaphore* joinSem;

    // Number of threads currently waiting for this thread to terminate
    // As long as there are any, the TCB is not deallocated because they must read its state
    uint64 joinCount;

    // Pointer used to link the thread into a queue
    TCB* next;

    template<typename T> friend class Queue;
    friend class Riscv;
    friend class MySemaphore;
    friend class SleepingThreadList;

    static void contextSwitch(Context* oldContext, Context* runningContext);

    // Takes the processor away from the current thread and gives it to the next ready thread
    static void dispatch();

    // A thread cannot free the stack it is running on, so terminated threads wait in this queue until another thread deallocates them
    static Queue<TCB> finishedQueue;
    static void collectFinished();

    static uint64 liveThreadCount;

    // Idle thread
    static TCB* idleThread;
    static void idleBody(void*);

    // Wrapper around the thread body
    static void threadWrapper();

    // Remaining time slice of the current thread
    static uint64 timeSliceCounter;
};

#endif