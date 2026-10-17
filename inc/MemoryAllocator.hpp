#ifndef MEMORY_ALLOCATOR_HPP
#define MEMORY_ALLOCATOR_HPP

#include "../lib/hw.h"

// Kernel memory allocator
class MemoryAllocator {
public:
    // Singleton
    static MemoryAllocator& getInstance();

    // Allocates a block of the specified size
    // Returns a pointer to the allocated memory or nullptr on failure
    void* memAlloc(size_t numOfBlocks);

    // Frees previously allocated memory
    // Returns SUCCESS or a negative error code on failure
    int memFree(void* address);

    // Helper - Calculates how many blocks are needed for a given number of bytes
    static size_t bytesToBlocks(size_t bytes) { return (bytes + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE; }

private:
    MemoryAllocator();

    // Delete the copy constructor and assignment operator
    MemoryAllocator(const MemoryAllocator&) = delete;
    MemoryAllocator& operator=(const MemoryAllocator&) = delete;

    // Segment header
    static const size_t HEADER_SIZE = MEM_BLOCK_SIZE;
    struct Segment {
        size_t size;
        Segment* next;
    };

    // Align absolute addresses so they are divisible by MEM_BLOCK_SIZE
    uint64 heapStart;
    uint64 heapEnd;

    // Insert into the free list, sorted by ascending address, merging adjacent segments
    void insertFree(Segment* seg);

    // List of free segments, sorted by ascending address
    Segment* freeHead;
};

#endif