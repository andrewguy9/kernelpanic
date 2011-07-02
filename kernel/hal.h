#ifndef HAL_H
#define HAL_H

#include"../utils/utils.h"

/*
 * Hal.h defines all of the interfaces that the rest of the kernel will 
 * consider as "hardware interfaces."
 */

//Includes for all builds
//#include"../utils/utils.h"
//#include"../utils/flags.h"

//STACK_INIT_ROUTINE is a routine which will be called to create a new
//stack entry in a new stack frame.
typedef void (STACK_INIT_ROUTINE) (void);

//MACHINE_CONTEXT is a structure which will be used to hold thead state
//between context switches. This context will be machine specific.
struct MACHINE_CONTEXT;

//ISR_HANDLER is the prototype all kernel interrupt handlers should use.
//they will be invoked either directly by hardware, or hal wrapper functionality.
typedef void (*ISR_HANDLER)();

enum IRQ_LEVEL {
    IRQ_LEVEL_NONE,
    IRQ_LEVEL_CRIT,
    IRQ_LEVEL_SOFT,
	IRQ_LEVEL_IO,
    IRQ_LEVEL_TIMER,
    IRQ_LEVEL_WATCHDOG,
	IRQ_LEVEL_COUNT, //The number of defined IRQs.

    IRQ_LEVEL_MAX = IRQ_LEVEL_TIMER,//Value of the highest atomic IRQ.
};

//
//Prototypes
//

void HalInitClock();
void HalWatchdogInit();
void HalContextStartup( STACK_INIT_ROUTINE * stackInitRoutine );
void HalIsrInit();
void HalRegisterIsrHandler( ISR_HANDLER handler, void * which, enum IRQ_LEVEL level );

void HalStartup();

void HalEnableWatchdog( int timeout );
void HalPetWatchdog( );
void HalStartInterrupt();
void HalEndInterrupt();
void HalCreateStackFrame( struct MACHINE_CONTEXT * Context, void * stack, STACK_INIT_ROUTINE foo, COUNT stackSize);
void HalGetInitialStackFrame( struct MACHINE_CONTEXT * Context );
void HalSerialStartup();
void HalPanic(char file[], int line);
void HalSleepProcessor();
void HalRaiseInterrupt(enum IRQ_LEVEL level);
void HalSetIrq(enum IRQ_LEVEL irq);

ATOMIC HalAtomicGetAndAnd(ATOMIC * var, ATOMIC val);
ATOMIC HalAtomicGetAndOr(ATOMIC * var, ATOMIC val);

#define HalAtomicGetAndSet(var) (HalAtomicGetAndOr(var, 1))
#define HalAtomicGetAndClear(var) (HalAtomicGetAndAnd(var, 0))

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
