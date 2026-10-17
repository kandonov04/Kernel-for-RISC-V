#include "../inc/SleepingThreadList.hpp"
#include "../inc/TCB.hpp"
#include "../inc/Scheduler.hpp"

TCB* SleepingThreadList::head = nullptr;

void SleepingThreadList::sleep(time_t periods) {
    if (periods == 0) {
        return;
    }

    TCB* caller = TCB::running;
    caller -> sleepTime = periods;
    caller -> state = TCB::SLEEPING;

    // Setting the relative time
    TCB** curr = &head;
    while (*curr && (*curr) -> sleepTime <= caller -> sleepTime) {
        caller -> sleepTime -= (*curr) -> sleepTime;
        curr = &(*curr) -> next;
    }
    caller -> next = *curr;
    if (*curr) {
        (*curr) -> sleepTime -= caller -> sleepTime;
    }

    *curr = caller;
    TCB::dispatch();
}

void SleepingThreadList::tick() {
    if (head == nullptr) {
        return;
    }
    // Only the first element is decremented - the others are expressed relatively
    head -> sleepTime--;

    // Wake up all threads at the beginning of the list whose time has expired
    while (head && head -> sleepTime == 0) {
        TCB* awake = head;
        head = head -> next;
        awake -> next = nullptr;
        awake -> state = TCB::READY;
        Scheduler::put(awake);
    }
}