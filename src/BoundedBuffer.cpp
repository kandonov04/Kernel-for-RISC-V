#include "../inc/BoundedBuffer.hpp"
#include "../inc/MySemaphore.hpp"
#include "../inc/MemoryAllocator.hpp"

BoundedBuffer::BoundedBuffer() : items(nullptr), capacity(0), head(0), tail(0), headMutex(nullptr), tailMutex(nullptr), spaceAvailable(nullptr), itemAvailable(nullptr) {}

bool BoundedBuffer::initialize(uint64 bufferCapacity) {
    if (bufferCapacity == 0) {
        return false;
    }
    items = (char*)MemoryAllocator::getInstance().memAlloc(MemoryAllocator::bytesToBlocks(bufferCapacity));
    if (items == nullptr) {
        return false;
    }
    capacity = bufferCapacity;
    head = 0;
    tail = 0;

    headMutex = MySemaphore::createSemaphore(1);
    tailMutex = MySemaphore::createSemaphore(1);
    spaceAvailable = MySemaphore::createSemaphore(capacity);
    itemAvailable = MySemaphore::createSemaphore(0);

    return headMutex != nullptr && tailMutex != nullptr && spaceAvailable != nullptr && itemAvailable != nullptr;
}

void BoundedBuffer::put(char item) {
    spaceAvailable -> wait(1);
    tailMutex -> wait(1);

    items[tail] = item;
    tail = (tail + 1) % capacity;

    tailMutex -> signal(1);
    itemAvailable -> signal(1);
}

char BoundedBuffer::get() {
    itemAvailable -> wait(1);
    headMutex -> wait(1);

    char item = items[head];
    head = (head + 1) % capacity;

    headMutex -> signal(1);
    spaceAvailable -> signal(1);

    return item;
}

// The interrupt handler must not block on anything
/*"If the buffer is full, the pressed characters can simply be discarded."*/
bool BoundedBuffer::putFromInterrupt(char item) {
    if (!spaceAvailable -> tryWait(1)) {
        return false;
    }

    items[tail] = item;
    tail = (tail + 1) % capacity;

    itemAvailable -> signal(1);

    return true;
}