#ifndef MY_SEMAPHORE_HPP
#define MY_SEMAPHORE_HPP

#include "../lib/hw.h"
#include "Queue.hpp"

class TCB;

// Semaphore in the kernel implementation
// Due to the wait_n and signal_n operations, the semaphore value is not a counter that goes negative and counts waiting threads, since each thread waits for a different number of units
// Value is therefore always the number of available units, and each blocked thread remembers how many it needs (TCB::units)
class MySemaphore {
public:
    void* operator new(size_t size) noexcept;
    void operator delete(void* address) noexcept;

    // Creates a semaphore with the specified initial value or returns nullptr on failure
    static MySemaphore* createSemaphore(uint64 init);

    ~MySemaphore();

    // Requests n resource units; if there are not enough, blocks the calling thread
    // Returns SUCCESS or ERR_SEM_DEALLOCATED if the semaphore is closed while the thread is waiting
    int wait(uint64 n);

    // Non-blocking version of the wait operation - immediately returns false if there are not enough units
    // Used by the interrupt handler, which must not block on anything
    bool tryWait(uint64 n);

    // Releases n units and wakes threads at the front of the queue while enough units are available
    int signal(uint64 n);

    // Unblocks all waiting threads - their wait operation returns an error
    int close();

private:
    explicit MySemaphore(uint64 init);

    // Delete the copy constructor and assignment operator
    MySemaphore(const MySemaphore&) = delete;
    MySemaphore& operator=(const MySemaphore&) = delete;

    uint64 value;
    bool closed;
    Queue<TCB> blockedQueue;
};

#endif