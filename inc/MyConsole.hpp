#ifndef MY_CONSOLE_HPP
#define MY_CONSOLE_HPP

#include "../lib/hw.h"
#include "BoundedBuffer.hpp"

// Output direction: the putc system call places a character in the output buffer, and a dedicated kernel thread transfers it from there to the controller by polling
// Input direction: the interrupt handler reads characters from the controller while any are available and places them in the input buffer, from which the getc system call retrieves them
// If the buffer is full, the character is discarded because the interrupt handler must not block!
class MyConsole {
public:
    // Singleton
    static MyConsole& getInstance();

    // Creates the buffer semaphores and starts the internal kernel thread
    // Called once during initialization, before interrupts are enabled
    static void initialize();

    void putChar(char chr);
    char getChar();

    // Retrieves a character from the output buffer for the internal kernel thread, blocking it if the buffer is empty
    char getOutputChar();

    // Handles an external console interrupt - receiving characters from the keyboard
    void handleInterrupt();

private:
    MyConsole() {}

    // Delete the copy constructor and assignment operator
    MyConsole(const MyConsole&) = delete;
    MyConsole& operator=(const MyConsole&) = delete;

    // Body of the internal kernel thread that drains the output buffer
    static void outputThreadBody(void*);

    // Buffer capacities
    static uint64 const OUTPUT_CAPACITY = 256;
    static uint64 const INPUT_CAPACITY = 128;

    BoundedBuffer outputBuffer;
    BoundedBuffer inputBuffer;
};

#endif