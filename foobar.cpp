/**
 * Foobar Module for Foobar Assignment
 *
 * This module contains the tasks required to fulfill the Foobar Assignment
 * brief. There are three tasks in this thread with the following
 * responsibilties:
 *
 * Foobar task:
 *   The foo task initializes or proceeds the foobar sequence. Its
 * responsibilities include determining if the current value to be printed is a
 * prime number and delivering the value to the correct thread to be printed.
 *
 * Foo task:
 *   Print out numbers sent to it by the Foobar task.
 *
 * Bar task:
 *   Similarly, this task only prints out numbers sent to it by the Foobar task.
 *
 * @author Jim Chong
 * @date   23 February 2024
 */

/**********************************************************
 *  Includes
 *********************************************************/
#include "foobar.hpp"

/* Standard C Libraries */
#include <string.h>

/* ESP32/Project Libraries */
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/**********************************************************
 *  Defines
 *********************************************************/
#define FOOBAR_TASK_PRIO (5)  //<! Task priorities
#define FOO_TASK_PRIO (6)
#define BAR_TASK_PRIO (6)

#define FOOBAR_TASK_STACK_SIZE (1024)
#define FOO_TASK_STACK_SIZE (1024)
#define BAR_TASK_STACK_SIZE (1024)

#define FOOBAR_QUEUE_SIZE (7)  //!< 7 queued + 1 active sequence
#define FOOBAR_MSG_SIZE (sizeof(uint32_t))

#define FOO_QUEUE_SIZE (1)
#define FOO_MSG_SIZE (sizeof(foobarMsg_t))

#define BAR_QUEUE_SIZE (1)
#define BAR_MSG_SIZE (sizeof(foobarMsg_t))

#define PRINT_BUFFER_SIZE (64)

/**********************************************************
 *  Struct Definitions
 *********************************************************/
/**
 * Structure holding the necessary data to print
 */
typedef struct {
    uint32_t value;  // value to print
    bool isPrime;
} foobarMsg_t;

/**********************************************************
 *  Static Variables
 *********************************************************/
// Foobar queue structure and handle
static StaticQueue_t staticFoobarQueue;
static QueueHandle_t foobarQueue;

// Foo and Bar queue structures and handles
static StaticQueue_t staticFooQueue;
static QueueHandle_t fooQueue;
static StaticQueue_t staticBarQueue;
static QueueHandle_t barQueue;

// Storage for message queues
static uint8_t foobarStorage[FOOBAR_QUEUE_SIZE * FOOBAR_MSG_SIZE];
static uint8_t fooStorage[FOO_QUEUE_SIZE * FOO_MSG_SIZE];
static uint8_t barStorage[BAR_QUEUE_SIZE * BAR_MSG_SIZE];

/**********************************************************
 *  Static Functions Declarations
 *********************************************************/
static void foobar_task(void* args);
static void foo_task(void* args);
static void bar_task(void* args);
static bool isPrime(uint32_t value);

/**********************************************************
 *  Function Definitions
 *********************************************************/
void foobar_initTasks(void) {
    xTaskCreatePinnedToCore(foobar_task, "foobar_task", FOOBAR_TASK_STACK_SIZE,
                            NULL, FOOBAR_TASK_PRIO, NULL, tskNO_AFFINITY);
}

QueueHandle_t foobar_getQueue(void) { return foobarQueue; }

/**********************************************************
 *  Static Function Definitions
 *********************************************************/

/**
 * The task runner for the Foobar task.
 *
 * This task will spawn the child Foo and Bar tasks, and then iterate through
 * its main routine once a second. During this routine the task will process the
 * current value to determine if it is a prime number, and which child process
 * will be responsible for printing. If no current value is set it will read its
 * own message queue to see if a request has been made to start the sequence.
 *
 * @param args initial parameters used in the task passed in at initialization
 */
static void foobar_task(void* args) {
    static uint32_t curValue = 0;

    foobarQueue = xQueueCreateStatic(FOOBAR_QUEUE_SIZE, FOOBAR_MSG_SIZE,
                                     foobarStorage, &staticFoobarQueue);

    // initialize child tasks
    xTaskCreatePinnedToCore(foo_task, "foo_task", FOO_TASK_STACK_SIZE, NULL,
                            FOOBAR_TASK_PRIO, NULL, PRO_CPU_NUM);
    xTaskCreatePinnedToCore(bar_task, "bar_task", BAR_TASK_STACK_SIZE, NULL,
                            FOOBAR_TASK_PRIO, NULL, APP_CPU_NUM);

    // main task loop
    while (1) {
        // if there is no sequence, attempt to start one by grabbing next value
        // in queue
        if (curValue == 0) {
            if (xQueueReceive(foobarQueue, (void*)&curValue, (TickType_t)0) !=
                pdPASS) {
                // reset just in case
                curValue = 0;
            }
        }

        // continue sequence if 'curValue' is set
        if (curValue > 0) {
            foobarMsg_t msg = {curValue, isPrime(curValue)};

            // send even numbers to foo
            if (curValue % 2 == 0) {
                if (xQueueSend(fooQueue, (void*)&msg, (TickType_t)0) !=
                    pdPASS) {
                    // log error
                }
            }
            // and then odd to bar
            else {
                if (xQueueSend(barQueue, (void*)&msg, (TickType_t)0) !=
                    pdPASS) {
                    // log error
                }
            }

            curValue--;
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

/**
 * The task runner for the Foo task.
 *
 * This task simply prints the Foo lines.
 *
 * @param args initial parameters used in the task passed in at initialization
 */
static void foo_task(void* args) {
    // initialize message queue
    fooQueue = xQueueCreateStatic(FOO_QUEUE_SIZE, FOO_MSG_SIZE, fooStorage,
                                  &staticFooQueue);

    while (1) {
        foobarMsg_t msg;

        // if queue contains a message, print data
        if (xQueueReceive(fooQueue, (void*)&msg, (TickType_t)0) != pdPASS) {
            char buffer[PRINT_BUFFER_SIZE];
            snprintf(buffer, 14, "Foo %ld",
                     msg.value);  // 14 holds the largest valid value

            // add Prime message if required
            if (msg.isPrime) {
                strncat(buffer, " Prime", 7);  // 7 is size of ' Prime\0'
            }

            uart_write_bytes(UART_NUM_1, buffer, strlen(buffer));
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

/**
 * The task runner for the Bar task.
 *
 * This task simply prints the Bar lines.
 *
 * @param args initial parameters used in the task passed in at initialization
 */
static void bar_task(void* args) {
    // initialize message queue
    barQueue = xQueueCreateStatic(BAR_QUEUE_SIZE, BAR_MSG_SIZE, barStorage,
                                  &staticBarQueue);

    while (1) {
        foobarMsg_t msg;

        // if queue contains a message, print data
        if (xQueueReceive(barQueue, (void*)&msg, (TickType_t)0) != pdPASS) {
            char buffer[PRINT_BUFFER_SIZE];
            snprintf(buffer, 14, "Bar %ld",
                     msg.value);  // 14 holds the largest valid value

            // add Prime message if required
            if (msg.isPrime) {
                strncat(buffer, " Prime", 7);  // 7 is size of ' Prime\0'
            }

            uart_write_bytes(UART_NUM_1, buffer, strlen(buffer));
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

/**
 * Determines if a number is prime
 *
 * @param value the value to scrutinize
 * @return true if the value passed in is a prime number
 */
static bool isPrime(uint32_t value) {
    bool retval = true;

    if (value <= 1) {
        retval = false;
    }

    for (uint32_t i = 2; i <= value / 2; i++) {
        if (value % i == 0) {
            retval = false;
        }
    }

    return retval;
}