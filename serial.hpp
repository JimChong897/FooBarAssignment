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
#ifndef __FOOBAR_SERIAL_H
#define __FOOBAR_SERIAL_H

/**********************************************************
 *  Functions Declarations
 *********************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Receives data from the serial port and forwards it onto the foobar task.
 */
void serial_postFoobarData(void);

#ifdef __cplusplus
}
#endif

#endif  // __FOOBAR_SERIAL_H