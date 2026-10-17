#ifndef SLEEPING_THREAD_LIST_HPP
#define SLEEPING_THREAD_LIST_HPP

#include "../lib/hw.h"

class TCB;

// List of sleeping threads, sorted by wake-up time
/*"Sleeping threads can be organized into a linked list sorted by wake-up time.
Each list element can store only the relative wake-up time with respect to the previous element in the list.
For the first element in the list, this value represents the time interval until its wake-up time relative to the current moment."*/
class SleepingThreadList {
public:
    // Puts the current thread to sleep for the specified number of timer periods
    // Returns only when the time has elapsed
    static void sleep(time_t periods);

    // Update on every timer period
    static void tick();

private:
    // Utility class - not instantiated
    SleepingThreadList() = delete;

    static TCB* head;
};

#endif