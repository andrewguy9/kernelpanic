#include "hal.h"
// This is a remote change!
/*
 * HAL UNIT DESCRIPTION
 *
 * The hal is responsible for all hardware dependant code.
 *
 * Rules:
 * The code for each hardware type should be surrounded with #ifdef ... #endif.
 *
 * All hardware dependant code should be placed in the hal unit.
 * Much of the hardware dependant code is impemented through macros.
 *
 * Functions prototypes should be hardware generic, placed once in the header
 * and implemented for each hardware type in the source file.
 *
 * Macros should be implemented in the header for each supported hardware type.
 *
 * Startup routines should be only called witih interrupts disabled,
 * so they dont have to gaurantee atomicy.
 */

#ifdef AVR_BUILD
#include"../hal/avr_hal.c"
#endif

#ifdef PC_BUILD
#include"../hal/unix_hal.c"
#endif

