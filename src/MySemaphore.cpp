#include "../inc/MySemaphore.hpp"
#include "../inc/TCB.hpp"
#include "../inc/Scheduler.hpp"
#include "../inc/Pool.hpp"
#include "../inc/Codes.hpp"

void* MySemaphore::operator new(size_t) noexcept {
    return Pool<MySemaphore>::allocate();
}

void MySemaphore::operator delete(void* address) noexcept {
    Pool<MySemaphore>::deallocate(address);
}

MySemaphore::MySemaphore(uint64 init) : value(init), closed(false) {}

MySemaphore::~MySemaphore() {
    if (!closed) {
        close();
    }
}

MySemaphore* MySemaphore::createSemaphore(uint64 init) {
    return new MySemaphore(init);
}

int MySemaphore::wait(uint64 n) {
    if (closed) {
        return ERR_SEM_DEALLOCATED;
    }
    if (n == 0) {
        return SUCCESS;
    }
    if (value >= n) {
        value -= n;
        return SUCCESS;
    }

    // Not enough units available - the thread is blocked
    TCB* caller = TCB::running;
    caller -> units = n;
    caller -> blockResult = SUCCESS;
    caller -> state = TCB::BLOCKED;
    blockedQueue.addLast(caller);

    TCB::dispatch();

    // Execution resumes here only when the thread gets the processor again
    return caller -> blockResult;
}

bool MySemaphore::tryWait(uint64 n) {
    if (closed) {
        return false;
    }
    if (n == 0) {
        return true;
    }
    if (value >= n) {
        value -= n;
        return true;
    }
    return false;
}

int MySemaphore::signal(uint64 n) {
    if (closed) {
        return ERR_SEM_DEALLOCATED;
    }
    value += n;

    // Wake up threads from the front of the queue as long as enough units are available - strict FIFO
    TCB* head;
    while ((head = blockedQueue.peekFirst()) && value >= head -> units) {
        blockedQueue.removeFirst();
        value -= head -> units;
        head -> units = 0;
        head -> blockResult = SUCCESS;
        head -> state = TCB::READY;
        Scheduler::put(head);
    }
    return SUCCESS;
}

int MySemaphore::close() {
    if (closed) {
        return ERR_SEM_DEALLOCATED;
    }
    closed = true;

    /*"All threads that are waiting on the semaphore are unblocked, and their wait operation returns an error."*/
    TCB* blocked;
    while ((blocked = blockedQueue.removeFirst())) {
        blocked -> units = 0;
        blocked -> blockResult = ERR_SEM_DEALLOCATED;
        blocked -> state = TCB::READY;
        Scheduler::put(blocked);
    }
    return SUCCESS;
}