#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include "Queue.hpp"

class TCB;

/*"The choice of scheduling algorithm is left entirely to the student.
Implementing the simplest algorithm, FIFO, is sufficient."*/
class Scheduler {
public:
    // Retrieves the first ready thread or returns nullptr if there are no ready threads
    static TCB* get();

    // Adds the thread to the end of the ready queue
    static void put(TCB* tcb);

private:
    // Utility class - not instantiated
    Scheduler() = delete;

    static Queue<TCB> readyThreadQueue;
};

#endif