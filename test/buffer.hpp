#ifndef BUFFER_CPP_H
#define BUFFER_CPP_H

#include "../inc/syscall_c.h"
#include "printing.hpp"

class Buffer {
private:
    int cap;
    int *buffer;
    int head, tail;

    sem_t spaceAvailable;
    sem_t itemAvailable;
    sem_t mutexHead;
    sem_t mutexTail;

public:
    Buffer(int _cap);
    ~Buffer();

    void put(int val);
    int get();

    int getCnt();

};


#endif

