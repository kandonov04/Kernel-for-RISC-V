#include "../inc/MemoryAllocator.hpp"
#include "../inc/Codes.hpp"

MemoryAllocator& MemoryAllocator::getInstance() {
    static MemoryAllocator instance;
    return instance;
}

MemoryAllocator::MemoryAllocator() : freeHead(nullptr) {
    heapStart = ((uint64)HEAP_START_ADDR + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE * MEM_BLOCK_SIZE;
    heapEnd = (uint64)HEAP_END_ADDR / MEM_BLOCK_SIZE * MEM_BLOCK_SIZE;

    // Initially, the entire heap is one free segment
    freeHead = (Segment*)heapStart;
    freeHead -> size = heapEnd - heapStart;
    freeHead -> next = nullptr;
}

void* MemoryAllocator::memAlloc(size_t numOfBlocks) {
    if (numOfBlocks == 0) {
        return nullptr;
    }

    if (numOfBlocks > ((size_t)-1 - HEADER_SIZE) / MEM_BLOCK_SIZE) {
        return nullptr;
    }

    // The segment must accommodate the requested space and its own header
    size_t segmentSize = HEADER_SIZE + numOfBlocks * MEM_BLOCK_SIZE;

    // First fit - the first sufficiently large segment is selected
    Segment* prev = nullptr;
    for (Segment* curr = freeHead; curr; prev = curr, curr = curr -> next) {
        if (curr -> size < segmentSize) {
            continue;
        }

        // The excess is split off as a new free segment, and curr is removed from the free list
        Segment* nextFree;

        if (curr -> size - segmentSize >= MEM_BLOCK_SIZE) {
            nextFree = (Segment*)((uint64)curr + segmentSize);
            nextFree -> size = curr -> size - segmentSize;
            nextFree -> next = curr -> next;
            curr -> size = segmentSize;
        } else {
            nextFree = curr -> next;
        }

        if (prev == nullptr) {
            freeHead = nextFree;
        } else {
            prev -> next = nextFree;
        }

        curr -> next = nullptr;

        // The address immediately after the header is returned to the user
        return (void*)((uint64)curr + HEADER_SIZE);
    }

    return nullptr;
}

int MemoryAllocator::memFree(void* address) {
    uint64 segmentValue = (uint64)address - HEADER_SIZE;
    Segment* segment = (Segment*)segmentValue;

    if (address == nullptr) {
        return ERR_NULL_ARG;
    }

    if (segmentValue < heapStart || segmentValue >= heapEnd || segmentValue % MEM_BLOCK_SIZE != 0) {
        return ERR_INVALID_ARG;
    }

    if (segment -> size == 0 || segment -> size % MEM_BLOCK_SIZE != 0 || segmentValue + segment -> size > heapEnd) {
        return ERR_INVALID_ARG;
    }

    for (Segment* curr = freeHead; curr; curr = curr -> next) {
        if (segmentValue >= (uint64)curr && segmentValue < (uint64)curr + curr -> size) {
            return ERR_FREE_FAILED;
        }
    }

    insertFree(segment);
    return SUCCESS;
}


void MemoryAllocator::insertFree(Segment* segment) {
    Segment* prev = nullptr;
    Segment* curr = freeHead;
    while (curr && curr < segment) {
        prev = curr;
        curr = curr -> next;
    }

    segment -> next = curr;
    if (prev == nullptr) {
        freeHead = segment;
    }
    else {
        prev -> next = segment;
    }

    if (curr && (uint64)segment + segment -> size == (uint64)curr) {
        segment -> size += curr -> size;
        segment -> next = curr -> next;
    }

    if (prev && (uint64)prev + prev -> size == (uint64)segment) {
        prev -> size += segment -> size;
        prev -> next = segment -> next;
    }
}