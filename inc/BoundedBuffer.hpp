#ifndef BOUNDED_BUFFER_HPP
#define BOUNDED_BUFFER_HPP

#include "../lib/hw.h"

class MySemaphore;

// Bounded character buffer
// The class has no destructor - the buffers live as long as the program itself
class BoundedBuffer {
public:
    BoundedBuffer();

    // Allocates memory and creates semaphores, returns false if allocation fails
    bool initialize(uint64 bufferCapacity);

    void put(char item);
    char get();

    // Insertion from the interrupt handler, returns false if the buffer is full
    bool putFromInterrupt(char item);

private:
    // Delete the copy constructor and assignment operator
    BoundedBuffer(const BoundedBuffer&) = delete;
    BoundedBuffer& operator=(const BoundedBuffer&) = delete;

    char* items;
    uint64 capacity;
    uint64 head;
    uint64 tail;

    MySemaphore* headMutex;
    MySemaphore* tailMutex;
    MySemaphore* spaceAvailable;
    MySemaphore* itemAvailable;
};

#endif