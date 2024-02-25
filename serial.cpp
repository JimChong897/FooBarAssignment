/**
 * Serial Module for Foobar Assignment
 *
 * This exposes a simple function to tie into an auxiliary thread that attempts
 * to read bytes from a serial UART connection and pass it onto the Foobar task.
 * This exists outside the Foobar task as the serial connection needs to be
 * served on a as-required basis, thus this function could be called immediately
 * after (but outside) of the UART ISR.
 *
 * @author Jim Chong
 * @date   23 February 2024
 */

/**********************************************************
 *  Includes
 *********************************************************/
#include "serial.hpp"

/* Standard C Libraries */
#include <cerrno>
#include <cstdlib>
#include <string.h>

/* ESP32/Project Libraries */
#include "driver/uart.h"
#include "foobar.hpp"

/**********************************************************
 *  Defines
 *********************************************************/
#define RX_BUF_SIZE (32)  //!< Buffer size to hold value

/**********************************************************
 *  Static Variables
 *********************************************************/
static uint8_t rxBuf[RX_BUF_SIZE];  //!< Receive data buffer

/**********************************************************
 *  Static Functions Declarations
 *********************************************************/
static bool serial_validateRxData(const uint8_t* buf, uint32_t* value);

/**********************************************************
 *  Constants
 *********************************************************/
const char* strInvalidData = "Invalid data";
const char* strQueueFull = "Currently full";

/**********************************************************
 *  Function Definitions
 *********************************************************/
void serial_postFoobarData(void) {
    // naive UART receive
    const int rxBytes = uart_read_bytes(UART_NUM_1, rxBuf, (RX_BUF_SIZE - 1),
                                        1000 / portTICK_PERIOD_MS);

    // if bytes were read, attempt to validate and process
    if (rxBytes > 0) {
        // terminate string
        rxBuf[rxBytes] = '\0';

        // check if value is a valid number
        uint32_t value;
        if (!serial_validateRxData(rxBuf, &value)) {
            // print invalid data message
            uart_write_bytes(UART_NUM_1, strInvalidData,
                             strlen(strInvalidData));
            return;
        } else {
            // print reception of valid request
            char strReceived[50] = "Received ";
            strncat(strReceived, (char*)rxBuf, strlen((char*)rxBuf));
            uart_write_bytes(UART_NUM_1, strReceived, strlen(strReceived));

            // reset system immediately if a '0' is received
            if (value == 0) {
                esp_restart();
            }

            // attempt to post to foobar task
            if (xQueueSend(foobar_getQueue(), (void*)&value, (TickType_t)0) !=
                pdPASS) {
                uart_write_bytes(UART_NUM_1, strQueueFull,
                                 strlen(strQueueFull));
            }
        }
    }
}

/**********************************************************
 *  Static Function Definitions
 *********************************************************/

/**
 * Determines if data in a buffer can completely be converted into an integer.
 * Note that the attempted conversion could cause a partial conversion to be
 * stored in 'value' and thus the return type should be scrutinized before using
 * the value passed.
 *
 * @param buf Buffer containing characters to test.
 * @param value Pointer to store converted value.
 * @return true if the data was completely and successfully converted to an
 * integer, otherwise false.
 */
static bool serial_validateRxData(const uint8_t* buf, uint32_t* value) {
    bool retval = false;
    char* strEnd = NULL;
    ;

    *value = strtol((char*)rxBuf, &strEnd, 10);

    if (errno == 0 && *strEnd == '\0') {
        retval = true;
    }

    return retval;
}