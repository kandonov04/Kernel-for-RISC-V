#include "../inc/Print.hpp"

// Direct write to the controller
// Does not go through the output buffer or the putc system call
// These functions are called from the interrupt handler when the system is already in an invalid state and nothing must be waited for
static void putcDirect(char chr) {
    volatile char* status = (volatile char*)CONSOLE_STATUS;
    volatile char* txData = (volatile char*)CONSOLE_TX_DATA;

    while ((*status & CONSOLE_TX_STATUS_BIT) == 0) {}
    *txData = chr;
}

void myPrintString(const char* string) {
    while (*string != '\0') {
        putcDirect(*string);
        string++;
    }
}

void myPrintNumber(uint64 number) {
    static const char digits[] = "0123456789ABCDEF";
    char buf[16];
    int i = 0;
    uint64 x = number;

    putcDirect('0');
    putcDirect('x');
    do {
        buf[i++] = digits[x % 16];
    } while ((x /= 16) != 0);

    while (--i >= 0) {
        putcDirect(buf[i]);
    }
}