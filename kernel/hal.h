#ifndef HAL_H
#define HAL_H

//Includes for all builds
#include"../utils/utils.h"
#include"../utils/flags.h"
//-----------------------------------------------------------------------------
//Defines (for all builds)
typedef void (*STACK_INIT_ROUTINE) (void);
//-----------------------------------------------------------------------------
//Prototypes ( For all Builds )
struct MACHINE_CONTEXT;
void HalInitClock();
void HalStartup();
void HalEnableWatchdog( int frequency );
void HalPetWatchdog( );
void HalStartInterrupt();
void HalEndInterrupt();
void HalCreateStackFrame( struct MACHINE_CONTEXT * Context, void * stack, STACK_INIT_ROUTINE foo, COUNT stackSize);
void HalGetInitialStackFrame( struct MACHINE_CONTEXT * Context );
void HalSerialStartup();
void HalPanic(char file[], int line);
void HalSleepProcessor();
//-----------------------------------------------------------------------------

#ifdef AVR_BUILD
#include"../hal/avr_hal.h"
#endif

#ifdef PC_BUILD
#include"../hal/unix_hal.h"
#endif

#endif //end of #ifndef HAL_H
