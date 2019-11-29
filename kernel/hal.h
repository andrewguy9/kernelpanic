#ifndef HAL_H
#define HAL_H

#include"utils/utils.h"

/*
 * Hal.h defines all of the interfaces that the rest of the kernel will 
 * consider as "hardware interfaces."
 */

//Includes for all builds
//#include"utils/utils.h"
//#include"utils/flags.h"

//STACK_INIT_ROUTINE is a routine which will be called to create a new
//stack entry in a new stack frame.
typedef void (STACK_INIT_ROUTINE) (void * arg);

//MACHINE_CONTEXT is a structure which will be used to hold thead state
//between context switches. This context will be machine specific.
struct MACHINE_CONTEXT;

enum IRQ_LEVEL {
    IRQ_LEVEL_NONE,
    IRQ_LEVEL_CRIT,
    IRQ_LEVEL_SOFT,
    IRQ_LEVEL_SERIAL_WRITE,
    IRQ_LEVEL_SERIAL_READ,
    IRQ_LEVEL_TIMER,
    IRQ_LEVEL_WATCHDOG,
    IRQ_LEVEL_COUNT, //The number of defined IRQs.

    IRQ_LEVEL_MAX = IRQ_LEVEL_TIMER,//Value of the highest atomic IRQ.
};

typedef void HAL_ISR_HANDLER(enum IRQ_LEVEL);

//
//Prototypes
//

void HalInitClock();
void HalSetTimer(TIME delta);
void HalWatchdogInit();
void HalIsrInit();
void HalRegisterIsrHandler( HAL_ISR_HANDLER handler, void * which, enum IRQ_LEVEL level );

void HalStartup();

void HalPetWatchdog( );
void HalStartInterrupt();
void HalEndInterrupt();
void HalCreateStackFrame( struct MACHINE_CONTEXT * Context, void * stack, COUNT stackSize, STACK_INIT_ROUTINE foo, void * arg);
void HalGetInitialStackFrame( struct MACHINE_CONTEXT * Context );
#define HalPanic(msg) HalPanicFn(__FILE__, __LINE__, msg)
void HalPanicFn(char file[], int line, char msg[]);
void HalError(char *fmt, ...);
void HalShutdownNow();
void HalSleepProcessor();
void HalRaiseInterrupt(enum IRQ_LEVEL level);
void HalSetIrq(enum IRQ_LEVEL irq);
void HalStartSerial();
_Bool HalSerialGetChar(char * out);
void HalSerialWriteChar(char data);
void * HalMap(char * tag, void * addr, COUNT len);

TIME HalGetTime();

//
//Include AVR only interfaces.
//

#ifdef AVR_BUILD
#include"../hal/avr_hal.h"
#endif

//
//Include UNIX only interfaces.
//

#ifdef PC_BUILD
#include"../hal/unix_hal.h"
#endif

#endif //end of #ifndef HAL_H
