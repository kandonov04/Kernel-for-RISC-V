#ifndef QUEUE_HPP
#define QUEUE_HPP

#include "../lib/hw.h"

/*"A more efficient solution is therefore to organize the pointers used for linking within the structure representing the thread itself,
but care must be taken to ensure that the same pointers are not used simultaneously to link the thread into different structures.
Care must also be taken to ensure that this implementation does not violate encapsulation."*/

template<typename T>
class Queue {
public:
    Queue() : head(nullptr), tail(nullptr), count(0) {}

    // Delete the copy constructor and assignment operator
    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;

    uint64 size() const { return count; }

    void addFirst(T* data) {
        data -> next = head;
        head = data;
        if (!tail) {
            tail = head;
        }
        count++;
    }

    void addLast(T* data) {
        data -> next = nullptr;
        if (tail) {
            tail -> next = data;
            tail = data;
        } else {
            head = tail = data;
        }
        count++;
    }

    T* removeFirst() {
        if (!head) {
            return nullptr;
        }
        T* elem = head;
        head = head -> next;
        if (!head) {
            tail = nullptr;
        }
        elem -> next = nullptr;
        count--;
        return elem;
    }

    T* removeLast() {
        if (!head) {
            return nullptr;
        }
        T* prev = nullptr;
        for (T* curr = head; curr && curr != tail; curr = curr -> next) {
            prev = curr;
        }
        T* elem = tail;
        if (prev) {
            prev -> next = nullptr;
        } else {
            head = nullptr;
        }
        tail = prev;
        elem -> next = nullptr;
        count--;
        return elem;
    }

    T* peekFirst() {
        return head;
    }

    T* peekLast() {
        return tail;
    }

private:
    T* head;
    T* tail;
    uint64 count;
};

#endif