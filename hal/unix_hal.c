#include"kernel/hal.h"
#include"kernel/thread.h"

#include<sys/time.h>
#include<string.h>
#include<signal.h>

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#include<errno.h>
#include<termios.h>

//-----------------------------------------------------------------------------
//-------------------------- GLOBALS ------------------------------------------
//-----------------------------------------------------------------------------

//
//Watchdog Variables
//

struct itimerval WatchdogInterval;

//
//Stack Management
//

STACK_INIT_ROUTINE * StackInitRoutine;

struct MACHINE_CONTEXT * halTempContext;
volatile BOOL halTempContextProcessed;


//
//Time Mangement
//

//
//IRQ Management
//

void HalIsrFinalize();

/*
 * This table establishes what handler to call and what signals to mask when a
 * signal is delivered.
 *
 * sigaction is the handler to call. This will normally be the generic handler: HalIsrHandler.
 * sa_mask is the mask to apply.
 * sa_flags is used for special masking rules/alt stack settings.
 */
struct sigaction HalIrqTable[IRQ_LEVEL_COUNT];

/*
 * HalIsrHandler uses this table to call the user specified handler.
 */
ISR_HANDLER * HalIsrJumpTable[IRQ_LEVEL_COUNT];

/*
 * HalIsrHandler uses this table to go from a signal to an IRQ.
 *
 * Key - IRQ level.
 * Value - Signal Number.
 *
 * HalIsrHandler will scan from the start of the array to the end.
 * When it finds the same signal number, then that index is the index it should call
 * from the HalIsrJumpTable.
 */
INDEX HalIrqToSignal[IRQ_LEVEL_COUNT];

//Create a mask for debugging
#ifdef DEBUG
sigset_t HalCurrrentIrqMask;
BOOL HalCurrrentIrqMaskValid;
#endif //DEBUG

//
//Serial Management
//

struct termios serialSettings;
struct termios serialSettingsOld;
int serialInFd;
int serialOutFd;

//-----------------------------------------------------------------------------
//--------------------------- HELPER PROTOTYPES -------------------------------
//-----------------------------------------------------------------------------

//
//Watchdog
//

//
//Stack Mangement
//

void HalStackTrampoline( int SignalNumber );

//
//Time Mangement
//

struct timeval HalStartupTime;

//
//IRQ Management
//

#ifdef DEBUG
void HalUpdateIsrDebugInfo();
void HalInvalidateIsrDebugInfo();
#endif
void HalClearSignals();
void HalBlockSignal( void * which );

//
//Serial Mangement
//

#define SERIAL_INPUT_DEVICE "/dev/tty"
#define SERIAL_OUTPUT_DEVICE "/dev/tty"

//-----------------------------------------------------------------------------
//------------------------- HELPER FUNCTIONS ----------------------------------
//-----------------------------------------------------------------------------

//
//Watchdog
//


//
//Stack Mangement
//

/*
 * sigaltstack will cause this function to be called on an alternate stack.
 * This allows us to bootstrap new threads.
 */
void HalStackTrampoline( int SignalNumber )
{
        int status;
        status = _setjmp( halTempContext->Registers );

        if( status == 0 ) {
                //Because status was 0 we know that this is the creation of
                //the stack frame. We can use the locals to construct the frame.

                halTempContextProcessed = TRUE;
                halTempContext = NULL;
                return;
        } else {
                //If we get here, then someone has jumped into a newly created thread.
                //Test to make sure we are atomic
                ASSERT( HalIsIrqAtomic(IRQ_LEVEL_TIMER) );

                StackInitRoutine();

                //Returning from a function which was invoked by siglongjmp is not
                //supported. Foo should never retrun.
                HalPanic("Tried to return from StackInitRoutine!\n", 0 );
                return;
        }
}

//
//Time Mangement
//

//
//IRQ Management
//

/*
 * All signals call this routine.
 * This routine then calls the appropriate ISR Handler.
 */
void HalIsrHandler( int SignalNumber )
{
        INDEX index;
        enum IRQ_LEVEL irq;

#ifdef DEBUG
        HalUpdateIsrDebugInfo();
#endif

        //We dont know which irq is associated with SignalNumber, so lets find it.
        for(index = 0; index < IRQ_LEVEL_COUNT; index++) {
                if( HalIrqToSignal[index] == SignalNumber ) {
                        //We found it, call the appropriate ISR.
                        irq = index;
                        HalIsrJumpTable[irq]();
#ifdef DEBUG
                        //We are about to return into an unknown frame.
                        //I can't predict what the irq will be there.
                        HalInvalidateIsrDebugInfo();
#endif
                        return;
                }
        }

        HalPanic("Signal delivered for which no Irq was registered", SignalNumber);
}

#ifdef DEBUG
void HalUpdateIsrDebugInfo()
{
        HalCurrrentIrqMaskValid = TRUE;
        sigprocmask(0, NULL, &HalCurrrentIrqMask);
}

void HalInvalidateIsrDebugInfo()
{
        HalCurrrentIrqMaskValid = FALSE;
}
#endif

/*
 * This is a unix specific hal function.
 * When starting up, we need to make sure that all
 * our IRQ tracking globals are initialized.
 */
void HalClearSignals()
{
        INDEX i;

        for(i=0; i < IRQ_LEVEL_COUNT; i++) {
                HalIrqTable[i].sa_handler = NULL;
                sigemptyset(&HalIrqTable[i].sa_mask);
                HalIrqTable[i].sa_flags = 0;
        }
}

/*
 * This is a unix specific hal function.
 * Some signals my be harmful to panic.
 * This function will block them for all IRQs.
 * Must be called before all HalRegisterIsrHandler calls.
 */
void HalBlockSignal( void * which )
{
        INDEX i;
        INDEX signum = (INDEX) which;

        for(i=0; i < IRQ_LEVEL_COUNT; i++) {
                sigaddset(&HalIrqTable[i].sa_mask, signum);
        }
}

//-----------------------------------------------------------------------------
//-------------------------- PUBLIC FUNCTIONS ---------------------------------
//-----------------------------------------------------------------------------

//
//Hal Utilities
//

void HalPanic(char file[], int line)
{
        printf("PANIC: %s:%d\n",file,line);
        abort();
}

void HalSleepProcessor()
{
        ASSERT( !HalIsIrqAtomic(IRQ_LEVEL_TIMER) );
        pause();
}

//
//Unit Management
//

void HalStartup()
{
}

//
//Watchdog
//

void HalPetWatchdog( TIME when )
{
        //NOTE: ITIMER_VIRUTAL will decrement when the process is running.
        //This means that on unix the watchdog will not catch cases where
        //the process is idle or sparse.

        //Note: calling HalPetWatchdog when the watchdog is disabled
        //is safe because setting it_intertval to zero keeps the dog
        //disabled.
        WatchdogInterval.it_interval.tv_sec = 0;
        WatchdogInterval.it_interval.tv_usec = 0;
        WatchdogInterval.it_value.tv_sec = 0;
        WatchdogInterval.it_value.tv_usec = when * 1000;
        ASSUME(setitimer( ITIMER_VIRTUAL, &WatchdogInterval, NULL ), 0);
}

//
//Stack Management
//

void HalContextStartup( STACK_INIT_ROUTINE * stackInitRoutine )
{
        StackInitRoutine = stackInitRoutine;
}

void HalCreateStackFrame(
                struct MACHINE_CONTEXT * Context,
                void * stack,
                STACK_INIT_ROUTINE foo,
                COUNT stackSize)
{
        char * cstack = stack;
        stack_t newStack;
        sigset_t oldSet;
        sigset_t trampolineMask;
        struct sigaction switchStackAction;

        sigemptyset( &trampolineMask );
        sigaddset( &trampolineMask, HAL_ISR_TRAMPOLINE );

#ifdef DEBUG
        //Set up the stack boundry.
        Context->High = (char *) (cstack + stackSize);
        Context->Low = cstack;
#endif

        Context->Foo = foo;

        //We are about to bootstrap the new thread. Because we have to modify global
        //state here, we must make sure no interrupts occur until after we are bootstrapped.
        //We do all of this under the nose of the Isr unit.
        sigprocmask(SIG_BLOCK, &HalIrqTable[IRQ_LEVEL_MAX].sa_mask, &oldSet);

        //NOTE: We use the interrupt mask here, because we want to block all operations.
        switchStackAction.sa_handler = HalStackTrampoline;
        switchStackAction.sa_mask = HalIrqTable[IRQ_LEVEL_MAX].sa_mask;
        switchStackAction.sa_flags = SA_ONSTACK;
        sigaction(HAL_ISR_TRAMPOLINE, &switchStackAction, NULL );

        halTempContext = Context;
        halTempContextProcessed = FALSE;

        newStack.ss_sp = cstack;
        newStack.ss_size = stackSize;
        newStack.ss_flags = 0;
        ASSUME(sigaltstack( &newStack, NULL ), 0);

        //At this point we know that we are atomic.
        //All signal types are blocked.
        //We will unblock the Trampoine signal, and make
        //sure that it was delivered.
        sigprocmask( SIG_UNBLOCK, &trampolineMask, NULL );

        raise( HAL_ISR_TRAMPOLINE );

        while( ! halTempContextProcessed );

        //Now that we have bootstrapped the new thread, lets restore the old mask.
        sigprocmask(SIG_SETMASK, &oldSet, NULL);
}

void HalGetInitialStackFrame( struct MACHINE_CONTEXT * Context )
{
#ifdef DEBUG
        int status = _setjmp( Context->Registers );
        ASSERT( status == 0 );//We should never wake here.
#else
        _setjmp( Context->Registers );
#endif

#ifdef DEBUG
        //The stack bounderies are infinite for the initial stack.
        Context->High = (char *) -1;
        Context->Low = (char *) 0;
#endif
}

//TODO: Add stack range check.
/*
 * Performs a context switch between one MACHINE_CONTEXT (thread) and another.
 * Because want to avoid having interrupts fire while the register file is in an intermidiate
 * state, all regilar interrupts should be disabled when calling this function.
 */
void HalContextSwitch(struct MACHINE_CONTEXT * oldStack, struct MACHINE_CONTEXT * newStack)
{
        int status;
        ASSERT( HalIsIrqAtomic(IRQ_LEVEL_MAX) );

        //Save the stack state into old context.
        status = _setjmp( oldStack->Registers );
        if( status == 0 )
        {
                //This was the saving call to setjmp.
                _longjmp( newStack->Registers, 1 );
        }
        else
        {
                //This was the restore call started by longjmp call.
                //We have just switched into a different thread.
        }

        ASSERT( HalIsIrqAtomic(IRQ_LEVEL_MAX) );
}

//
//Time Management
//

/*
 * Calculates the difference between two times in milliseconds.
 * Time1 is supposed to be before time2.
 * If time1 appears to be after time2, HalTimeDelta will return 0 (same time).
 */
TIME HalTimeDelta(struct timeval *time1, struct timeval *time2)
{
        TIME delta = 0;

        if(time2->tv_sec < time1->tv_sec) {
                return 0;
        } else if(time2->tv_sec == time1->tv_sec && time2->tv_usec < time1->tv_usec) {
                return 0;
        } else {
                delta += (time2->tv_sec  - time1->tv_sec)  * 1000; // Seconds * 1000 = Milliseconds
                delta += (time2->tv_usec - time1->tv_usec) / 1000; // Microseconds / 1000 = Milliseconds

                return delta;
        }
}

void HalInitClock()
{
        //Set the startup time.
        ASSUME(gettimeofday(&HalStartupTime, NULL), 0);
}

void HalSetTimer(TIME delta)
{
        struct itimerval TimerInterval;

        time_t seconds = delta / 1000;
        time_t mills = delta % 1000;
        time_t micros = mills * 1000;

        if(seconds > 100000000) {
                seconds = 100000000;
        }

        if(seconds ==0 && micros == 0) {
                micros = 1000;
        }

        //Set the timer interval.
        TimerInterval.it_interval.tv_sec = 0;
        TimerInterval.it_interval.tv_usec = 0;
        TimerInterval.it_value.tv_sec = seconds;
        TimerInterval.it_value.tv_usec = micros;

        ASSUME(setitimer( ITIMER_REAL, &TimerInterval, NULL ), 0);
}

void HalResetClock()
{
        //Note: On Unix we dont do anything here because
        //the timer is already periodic, unlike the avr.
}

TIME HalGetTime()
{
        struct timeval sysTime;
        ASSUME(gettimeofday(&sysTime, NULL), 0);

        return HalTimeDelta(&HalStartupTime, &sysTime);
}

//
//IRQ Management
//

#define HACK 1

#ifdef DEBUG
#if HACK
#ifdef LINUX
sigset_t sigset_xor(sigset_t a, sigset_t b) {
	sigset_t result;
	for (int i = 0; i < _SIGSET_NWORDS; i++) {
		result.__val[i] = a.__val[i] ^ b.__val[i];
	}
	return result;
}

sigset_t sigset_and(sigset_t a, sigset_t b) {
	sigset_t result;
	for (int i = 0; i< _SIGSET_NWORDS; i++) {
		result.__val[i] = a.__val[i] & b.__val[i];
	}
	return result;
}

BOOL sigset_empty(sigset_t a) {
	for (int i = 0; i< _SIGSET_NWORDS; i++) {
		if (a.__val[i] != 0) {
			return FALSE;
		}
	}
	return TRUE;
}
#else
sigset_t sigset_xor(sigset_t a, sigset_t b) {
	return a ^ b;
}
sigset_t sigset_and(sigset_t a, sigset_t b) {
	return a & b;
}

BOOL sigset_empty(sigset_t a) {
	return !a;
}
#endif
#endif //HACK
/*
 * Returns true if the system is running at at least IRQ level.
 */
BOOL HalIsIrqAtomic(enum IRQ_LEVEL level)
{
        sigset_t curSet;
        int status;

        status = sigprocmask(0, NULL, &curSet);
        ASSERT(status == 0);

        HalUpdateIsrDebugInfo();
	
	// empty -> 0, !0 -> true
	// not empty -> !0, !!0 -> false
#if HACK
        return sigset_empty(sigset_and(sigset_xor(HalIrqTable[level].sa_mask,curSet), HalIrqTable[level].sa_mask));
#else
	return !((HalIrqTable[level].sa_mask ^ curSet) & HalIrqTable[level].sa_mask);
#endif
}
#endif //DEBUG

void HalSetIrq(enum IRQ_LEVEL irq)
{
        sigprocmask( SIG_SETMASK, &HalIrqTable[irq].sa_mask, NULL);

#ifdef DEBUG
        HalUpdateIsrDebugInfo();
#endif
}

void HalRaiseInterrupt(enum IRQ_LEVEL level)
{
        raise( HalIrqToSignal[level] );
}

void HalIsrInit()
{
        HalClearSignals();

        //Unix Hal requires uses HAL_ISR_TRAMPOLINE to bootstrap new
        //thread stacks. We need to ensure that it is blocked by all
        //IRQ levels.
        HalBlockSignal( (void *) HAL_ISR_TRAMPOLINE );
}

/*
 * IRQ based systems typically have a reserved space in memory which
 * serves as a jump table. The table is filled with function pointers
 * to invoke when various irq level transitions occur. Typically different
 * offsets also get a level associated with it. So that you can change the
 * order of various devices. (so a can mask c and b can mask c.
 *
 * handler - the function pointer to invoke.
 * which - the location which indicates what hardware event happed.
 * level - what irq to assign to the hardware event.
 */
void HalRegisterIsrHandler( ISR_HANDLER handler, void * which, enum IRQ_LEVEL level)
{

        INDEX i;
        INDEX signum = (INDEX) which;

        ASSERT(HalIsIrqAtomic(IRQ_LEVEL_MAX));

        for(i=level; i < IRQ_LEVEL_COUNT; i++) {
                sigaddset(&HalIrqTable[i].sa_mask, signum);
        }

        HalIrqToSignal[level] = signum;
        HalIsrJumpTable[level] = handler;
        HalIrqTable[level].sa_handler = HalIsrHandler;

        HalIsrFinalize();
        HalSetIrq(IRQ_LEVEL_MAX);
}

void HalIsrFinalize()
{
        enum IRQ_LEVEL level;

        for(level = IRQ_LEVEL_NONE; level < IRQ_LEVEL_COUNT; level++) {
                if(HalIrqToSignal[level] != 0) {
                        ASSUME( sigaction(HalIrqToSignal[level], &HalIrqTable[level], NULL), 0 );
                }
        }
}

//
// Serial Mangement
//

void HalStartSerial()
{
        int oflags;

        // Open the term for reading and writing.
        if( (serialInFd = open(SERIAL_INPUT_DEVICE, O_RDWR | O_NOCTTY | O_NONBLOCK)) < 0 ) {
                HalPanic("failed to open input fd", errno);
        }
        if( (serialOutFd = open(SERIAL_OUTPUT_DEVICE, O_WRONLY | O_NOCTTY | O_NONBLOCK) ) < 0 ) {
                HalPanic("failed to open output fd", errno);
        }

        //Get the term settings
        tcgetattr(serialInFd, &serialSettingsOld);

        // For now, lets reuse whatever terminal settings we were started with.
        serialSettings.c_cflag = serialSettingsOld.c_cflag;
        serialSettings.c_iflag = serialSettingsOld.c_iflag;
        serialSettings.c_oflag = serialSettingsOld.c_oflag;
        serialSettings.c_lflag = serialSettingsOld.c_lflag;

        serialSettings.c_cc[VEOF] = serialSettingsOld.c_cc[VEOF];
        serialSettings.c_cc[VEOL] = serialSettingsOld.c_cc[VEOL];
        serialSettings.c_cc[VEOL2] = serialSettingsOld.c_cc[VEOL2];
        serialSettings.c_cc[VERASE] = serialSettingsOld.c_cc[VERASE];
        serialSettings.c_cc[VWERASE] = serialSettingsOld.c_cc[VWERASE];
        serialSettings.c_cc[VKILL] = serialSettingsOld.c_cc[VKILL];
        serialSettings.c_cc[VREPRINT] = serialSettingsOld.c_cc[VREPRINT];
        serialSettings.c_cc[VINTR] = serialSettingsOld.c_cc[VINTR];
        serialSettings.c_cc[VQUIT] = serialSettingsOld.c_cc[VQUIT];
        serialSettings.c_cc[VSUSP] = serialSettingsOld.c_cc[VSUSP];
#ifndef LINUX
	serialSettings.c_cc[VDSUSP] = serialSettingsOld.c_cc[VDSUSP];
#endif
        serialSettings.c_cc[VSTART] = serialSettingsOld.c_cc[VSTART];
        serialSettings.c_cc[VSTOP] = serialSettingsOld.c_cc[VSTOP];
        serialSettings.c_cc[VLNEXT] = serialSettingsOld.c_cc[VLNEXT];
        serialSettings.c_cc[VDISCARD] = serialSettingsOld.c_cc[VDISCARD];
        serialSettings.c_cc[VMIN] = serialSettingsOld.c_cc[VMIN];
        serialSettings.c_cc[VTIME] = serialSettingsOld.c_cc[VTIME];
#ifndef LINUX
	serialSettings.c_cc[VSTATUS] = serialSettingsOld.c_cc[VSTATUS];
#endif

        serialSettings.c_ispeed = serialSettingsOld.c_ispeed;
        serialSettings.c_ospeed = serialSettingsOld.c_ospeed;

        tcflush(serialInFd, TCIFLUSH);
        tcsetattr(serialInFd,TCSANOW,&serialSettings);

        fcntl(serialInFd, F_SETOWN, getpid(  ));
        oflags = fcntl(serialInFd, F_GETFL);
        fcntl(serialInFd, F_SETFL, oflags | FASYNC);

        fcntl(serialOutFd, F_SETOWN, getpid(  ));
        oflags = fcntl(serialOutFd, F_GETFL);
        fcntl(serialOutFd, F_SETFL, oflags | FASYNC);
}

BOOL HalSerialGetChar(char * out)
{
        int readlen = read(serialInFd, out, sizeof(char));

        if(readlen > 0) {
                return TRUE;
        } else if(readlen == 0) {
                //We are allowed to recieve zero bytes from the serial.
                return FALSE;
        } else {
                if(errno == EINTR) {
                        return FALSE; //We are allowed to be interrupted by another signal.
                } else if(errno == EAGAIN) {
                        return FALSE;
                } else if(errno == EWOULDBLOCK) {
                        return FALSE;
                } else {
                        HalPanic("Recieved error from STDIN!\n", errno );
                        return FALSE;
                }
        }
}

void HalSerialWriteChar(char data)
{
        int writelen = write(serialOutFd, &data, sizeof(char));

        if( writelen > 0 ) {

        } else if(writelen == 0) {
                HalPanic("Wrote 0 to STDOUT\n", 0);
        } else {
                HalPanic("Failed to write to STDOUT", errno);
        }
}
