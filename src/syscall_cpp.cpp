#include "../inc/syscall_cpp.hpp"
#include "../inc/Codes.hpp"

void* operator new(size_t size) {
    return mem_alloc(size);
}

void operator delete(void* address) noexcept {
    mem_free(address);
}

void* operator new[](size_t size) {
    return mem_alloc(size);
}

void operator delete[](void* address) noexcept {
    mem_free(address);
}

Thread::Thread(void(*body)(void*), void* arg) : myHandle(nullptr), body(body), arg(arg) {}

Thread::Thread() : myHandle(nullptr), body(nullptr), arg(nullptr) {}

Thread::~Thread() {
    // The kernel deallocates both the stack and its thread structure when the thread terminates, so there is nothing to deallocate here
}

void Thread::wrapper(void* threadObject) {
    Thread* thread = (Thread*)threadObject;

    /*"If a function pointer was set by the constructor, the run operation must be ignored in all cases."*/
    if (thread -> body) {
        thread -> body(thread -> arg);
    } else {
        thread -> run();
    }
}

int Thread::start() {
    if (myHandle) {
        return ERR_INVALID_HANDLE;
    }

    return thread_create(&myHandle, &Thread::wrapper, this);
}

void Thread::dispatch() {
    thread_dispatch();
}

int Thread::sleep(time_t periods) {
    return time_sleep(periods);
}

int Thread::getId() {
    return thread_get_id();
}

void Thread::join() {
    thread_join(&myHandle);
}

Semaphore::Semaphore(unsigned init) : myHandle(nullptr) {
    sem_open(&myHandle, init);
}

Semaphore::~Semaphore() {
    if (myHandle != nullptr) {
        sem_close(myHandle);
        myHandle = nullptr;
    }
}

int Semaphore::wait() {
    return sem_wait(myHandle);
}

int Semaphore::signal() {
    return sem_signal(myHandle);
}

PeriodicThread::PeriodicThread(time_t period) : period(period) {}

// Termination is signaled by setting the period to zero
void PeriodicThread::terminate() {
    period = 0;
}

void PeriodicThread::run() {
    while (period > 0) {
        periodicActivation();

        // The period is checked again because terminate may have been called from periodicActivation itself or from another thread
        if (period > 0) {
            time_sleep(period);
        }
    }
}

char Console::getc() {
    return ::getc();
}

void Console::putc(char chr) {
    ::putc(chr);
}