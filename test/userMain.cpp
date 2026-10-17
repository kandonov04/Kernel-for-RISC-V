#include "printing.hpp"
#include "../test/Threads_C_API_test.hpp"
#include "../test/Threads_CPP_API_test.hpp"
#include "../test/System_Mode_test.hpp"
#include "../test/ConsumerProducer_C_API_test.hpp"
#include "../test/ConsumerProducer_CPP_Sync_API_test.hpp"
#include "../test/ThreadSleep_C_API_test.hpp"
#include "../test/ConsumerProducer_CPP_API_test.hpp"
#include "System_Mode_test.hpp"

void userMain() {
    printString("Enter the test number? [1-7]\n");
    int test = getc() - '0';
    getc();

    switch (test) {
        case 1:
            Threads_C_API_test();
            printString("TEST 1 (C API threads and synchronous context switching)\n");
            break;
        case 2:
            Threads_CPP_API_test();
            printString("TEST 2 (C++ API threads and synchronous context switching)\n");
            break;
        case 3:
            producerConsumer_C_API();
            printString("TEST 3 (Complete C API with semaphores and synchronous context switching)\n");
            break;
        case 4:
            producerConsumer_CPP_Sync_API();
            printString("TEST 4 (Complete C++ API with semaphores and synchronous context switching)\n");
            break;
        case 5:
            testSleeping();
            printString("TEST 5 (C API thread_sleep test)\n");
            break;
        case 6:
            testConsumerProducer();
            printString("TEST 6 (C++ API and asynchronous context switching)\n");
            break;
        case 7:
            System_Mode_test();
            printString("The test did not complete successfully\n");
            printString("TEST 7 (Testing whether user code executes in user mode)\n");
            break;
        default:
            printString("You did not enter a valid test number\n");
    }
}