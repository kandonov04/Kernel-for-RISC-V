#ifndef POOL_HPP
#define POOL_HPP

#include "../lib/hw.h"
#include "MemoryAllocator.hpp"

// Allocation of small kernel objects
/*"Dynamic allocation. This solution assumes dynamic allocation of arrays
using the kernel memory allocator as needed, and then using these arrays
as if they were statically allocated. It is useful to separate the described
functionality of allocating slots and arrays into a separate class, specifically
a template class, which makes it easier to use in different places and classes."*/
template<typename T, uint64 SLOTS_PER_CHUNK = 16>
class Pool {
public:
    // Returns memory for one object or nullptr if no more memory is available
    static void* allocate() {
        if (freeHead == nullptr && !grow()) {
            return nullptr;
        }
        Slot* slot = freeHead;
        freeHead = slot -> next;
        return slot;
    }

    static void deallocate(void* address) {
        if (address == nullptr) {
            return;
        }
        Slot* slot = (Slot*)address;
        slot -> next = freeHead;
        freeHead = slot;
    }

private:
    // Utility class - not instantiated
    Pool() = delete;

    // While a slot is free, it holds a pointer to the next slot; while occupied, it holds an object
    union Slot {
        Slot* next;
        uint64 storage[(sizeof(T) + sizeof(uint64) - 1) / sizeof(uint64)];
    };

    // Allocates a new array of slots and links them into the free list
    static bool grow() {
        void* chunk = MemoryAllocator::getInstance().memAlloc(MemoryAllocator::bytesToBlocks(sizeof(Slot) * SLOTS_PER_CHUNK));
        if (chunk == nullptr) {
            return false;
        }
        Slot* slots = (Slot*)chunk;
        for (uint64 i = 0; i < SLOTS_PER_CHUNK; i++) {
            slots[i].next = freeHead;
            freeHead = &slots[i];
        }
        return true;
    }

    static Slot* freeHead;
};

template<typename T, uint64 SLOTS_PER_CHUNK>
typename Pool<T, SLOTS_PER_CHUNK>::Slot* Pool<T, SLOTS_PER_CHUNK>::freeHead = nullptr;

#endif