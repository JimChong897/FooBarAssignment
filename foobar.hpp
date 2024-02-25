/**
 * Foobar Module for Foobar Assignment
 *
 * This module contains the tasks required to fulfill the Foobar Assignment
 * brief. This header file exposes only the functions required to interact with
 * the tasks once spawned. It is the responsibility of the data handler, in this
 * case the serial module, to push data to the foobar thread via a message
 * queue. Thus, only the message queue is exposed.
 *
 * @author Jim Chong
 * @date   23 February 2024
 */
#ifndef __FOOBAR_H
#define __FOOBAR_H

/**********************************************************
 *  Includes
 *********************************************************/
#include "freertos/FreeRTOS.h"

/**********************************************************
 *  Functions Declarations
 *********************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates the foobar tasks and initializes associated message queues.
 */
void foobar_initTasks(void);

/**
 * Returns the handle to the message queue used to communicate new data requests
 * to the foobar task.
 *
 * @return The handle to the message queue.
 */
QueueHandle_t foobar_getQueue(void);

#ifdef __cplusplus
}
#endif

#endif  // __FOOBAR_H
