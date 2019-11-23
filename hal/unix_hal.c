#include"kernel/hal.h"
#include"kernel/thread.h"

#include<sys/time.h>
#include<string.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#include<errno.h>
#include<termios.h>
#include<sys/mman.h>
#include<stdarg.h>

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
struct sigaction HalIrqToSigaction[IRQ_LEVEL_COUNT];
INDEX HalIrqToSignal[IRQ_LEVEL_COUNT];

HAL_ISR_HANDLER * HalSignalToHandler[NSIG];
enum IRQ_LEVEL HalSignalToIrq[NSIG];

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
        //Save stack startup state before releaseing the tempContext.
        STACK_INIT_ROUTINE * foo = halTempContext->Foo;
        void * arg = halTempContext->Arg;

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
                ASSERT( HalIsIrqAtomic(IRQ_LEVEL_MAX) );

                foo(arg);

                //Returning from a function which was invoked by siglongjmp is not
                //supported. Foo should never retrun.
                HalPanic("Tried to return from trampoline!");
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
        enum IRQ_LEVEL irq = HalSignalToIrq[SignalNumber];
        HAL_ISR_HANDLER * handler = HalSignalToHandler[SignalNumber];

        ASSERT (SignalNumber >= 0 && SignalNumber < NSIG);
        if (handler == NULL) {
                HalPanic("Signal delivered for which no Irq was registered");
        }
        ASSERT (handler != NULL);
        ASSERT (irq != IRQ_LEVEL_NONE);
#ifdef DEBUG
        HalUpdateIsrDebugInfo();
#endif
        handler(irq);
#ifdef DEBUG
        //We are about to return into an unknown frame.
        //I can't predict what the irq will be there.
        HalInvalidateIsrDebugInfo();
#endif
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
                HalIrqToSignal[i] = 0;
                HalIrqToSigaction[i].sa_handler = NULL;
                sigemptyset(&HalIrqToSigaction[i].sa_mask);
                HalIrqToSigaction[i].sa_flags = 0;
        }

        for(i=0; i<NSIG; i++) {
                HalSignalToHandler[i] = NULL;
                HalSignalToIrq[i] = IRQ_LEVEL_NONE;
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
                sigaddset(&HalIrqToSigaction[i].sa_mask, signum);
        }
}

//-----------------------------------------------------------------------------
//-------------------------- PUBLIC FUNCTIONS ---------------------------------
//-----------------------------------------------------------------------------

//
//Hal Utilities
//

void HalPanicFn(char file[], int line, char msg[])
{
  HalError("PANIC: %s:%d %s\n", file, line, msg);
}

#define HalPanicErrno(msg) HalPanicErrnoFn(__FILE__, __LINE__, msg)

void HalPanicErrnoFn(char file[], int line, char msg[])
{
  HalError("PANIC: %s:%d errno %s: %s\n", file, line, strerror(errno), msg);
}

void HalError(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
  abort();
}

void HalShutdownNow() {
  exit(0);
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

void HalCreateStackFrame(
                struct MACHINE_CONTEXT * Context,
                void * stack,
                COUNT stackSize,
                STACK_INIT_ROUTINE foo,
                void * arg)
{
        int status;
        char * cstack = stack;
        stack_t newStack;
        sigset_t oldSet;
        sigset_t trampolineMask;
        struct sigaction switchStackAction;

        ASSUME(sigemptyset( &oldSet ), 0);
        ASSUME(sigemptyset( &trampolineMask ), 0);
        ASSUME(sigaddset( &trampolineMask, HAL_ISR_TRAMPOLINE ), 0);

        Context->Foo = foo;
        Context->Arg = arg;

        //We are about to bootstrap the new thread. Because we have to modify global
        //state here, we must make sure no interrupts occur until after we are bootstrapped.
        //We do all of this under the nose of the Isr unit.
        sigprocmask(SIG_BLOCK, &HalIrqToSigaction[IRQ_LEVEL_MAX].sa_mask, &oldSet);

        //NOTE: We use the interrupt mask here, because we want to block all operations.
        switchStackAction.sa_handler = HalStackTrampoline;
        switchStackAction.sa_mask = HalIrqToSigaction[IRQ_LEVEL_MAX].sa_mask;
        switchStackAction.sa_flags = SA_ONSTACK;
        sigaction(HAL_ISR_TRAMPOLINE, &switchStackAction, NULL );

        halTempContext = Context;
        halTempContextProcessed = FALSE;

        newStack.ss_sp = cstack;
        newStack.ss_size = stackSize;
        newStack.ss_flags = 0;
        status = sigaltstack( &newStack, NULL );
	if (status != 0) {
		HalPanicErrno("Failed to turn on sigaltstack.");
	}


        status = raise( HAL_ISR_TRAMPOLINE );
        if (status != 0) {
                HalPanicErrno("Failed raise stack bootstrap signal");
        }

        //At this point we know that we can't be interrupted.
        //The trampoline signal has been triggered.
        //All signals are blocked.
        //We will unblock the Trampoine signal so it gets delivered.
        ASSUME(sigprocmask( SIG_UNBLOCK, &trampolineMask, NULL ), 0);

        //Make sure that the signal was delivered.
        if (!halTempContextProcessed) {
                HalPanic("Failed to bootstrap new stack via signal");
        }

	//Now that trampoline has fired, we can get back to the thread with longjump.
	//Lets turn off sigaltstack.
        newStack.ss_flags = SS_DISABLE;
        status = sigaltstack( &newStack, NULL );
	if (status != 0) {
		HalPanicErrno("Failed to turn off sigaltstack.");
	}

        //Now that we have bootstrapped the new thread, lets restore the old mask.
        ASSUME(sigprocmask(SIG_SETMASK, &oldSet, NULL), 0);
}

void HalGetInitialStackFrame( struct MACHINE_CONTEXT * Context )
{
#ifdef DEBUG
        int status = _setjmp( Context->Registers );
        ASSERT( status == 0 );//We should never wake here.
#else
        _setjmp( Context->Registers );
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

#undef SIGNAL_HACK

#ifdef DEBUG
#ifdef LINUX
sigset_t sigset_and(sigset_t a, sigset_t b) {
	int status;
	sigset_t result;
	status = sigemptyset(&result);
	ASSUME(status, 0);
	status = sigandset(&result, &a, &b);
	ASSUME(status, 0);
	return result;
}

sigset_t sigset_or(sigset_t a, sigset_t b) {
	int status;
	sigset_t result;
	status = sigemptyset(&result);
	ASSUME(status, 0);
	status = sigorset(&result, &a, &b);
	ASSUME(status, 0);
	return result;
}

BOOL sigset_empty(sigset_t a) {
	return sigisemptyset(&a);
}
#ifdef SIGNAL_HACK // Use function which touch linux struct internals.
sigset_t sigset_xor(sigset_t a, sigset_t b) {
	sigset_t result;
	for (int i = 0; i < _SIGSET_NWORDS; i++) {
		result.__val[i] = a.__val[i] ^ b.__val[i];
	}
	return result;
}
#else // Use linux singal interface only.
sigset_t sigset_not(sigset_t a) {
	int i;
	int status;
	sigset_t result;
	status = sigemptyset(&result);
	ASSUME(status, 0);
        for (i=1; i <= __SIGRTMAX; i++) {
                status = sigismember(&a, i);
                if (status == 0) {
                  //Not set, so set in result.
                  status = sigaddset(&result, i);
                  ASSUME(status, 0);
                } else if (status == 1) {
                  //Set, so unset.
                  status = sigdelset(&result, i);
                  ASSUME(status, 0);
                } else if (status == -1) {
                  HalPanicErrno("Failed to test signal membership.");
                }
        }
        return result;
}

sigset_t sigset_xor(sigset_t a, sigset_t b) {
        sigset_t result;
        result = sigset_and( sigset_not( sigset_and(a, b)), sigset_or(a, b));
        return result;
}
#endif // SIGNAL_HACK

#else // OSX
sigset_t sigset_xor(sigset_t a, sigset_t b) {
	return a ^ b;
}
sigset_t sigset_and(sigset_t a, sigset_t b) {
	return a & b;
}

BOOL sigset_empty(sigset_t a) {
	return !a;
}
#endif // LINUX
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

        return sigset_empty(sigset_and(sigset_xor(HalIrqToSigaction[level].sa_mask,curSet), HalIrqToSigaction[level].sa_mask));
}
#endif //DEBUG

void HalSetIrq(enum IRQ_LEVEL irq)
{
        sigprocmask( SIG_SETMASK, &HalIrqToSigaction[irq].sa_mask, NULL);

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
void HalRegisterIsrHandler( HAL_ISR_HANDLER handler, void * which, enum IRQ_LEVEL level)
{

        INDEX i;
        INDEX signum = (INDEX) which;

        ASSERT(HalIsIrqAtomic(IRQ_LEVEL_MAX));

        for(i=level; i < IRQ_LEVEL_COUNT; i++) {
                sigaddset(&HalIrqToSigaction[i].sa_mask, signum);
        }

        HalIrqToSignal[level] = signum;
        HalSignalToHandler[signum] = handler;
        HalSignalToIrq[signum] = level;
        HalIrqToSigaction[level].sa_handler = HalIsrHandler;

        HalIsrFinalize();
        HalSetIrq(IRQ_LEVEL_MAX);
}

void HalIsrFinalize()
{
        enum IRQ_LEVEL level;

        for(level = IRQ_LEVEL_NONE; level < IRQ_LEVEL_COUNT; level++) {
                if(HalIrqToSignal[level] != 0) {
                        ASSUME( sigaction(HalIrqToSignal[level], &HalIrqToSigaction[level], NULL), 0 );
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
                HalPanicErrno("failed to open input fd");
        }
        if( (serialOutFd = open(SERIAL_OUTPUT_DEVICE, O_WRONLY | O_NOCTTY | O_NONBLOCK) ) < 0 ) {
                HalPanicErrno("failed to open output fd");
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
                        HalPanicErrno("Recieved error from STDIN!");
                        return FALSE;
                }
        }
}

void HalSerialWriteChar(char data)
{
        int writelen = write(serialOutFd, &data, sizeof(char));

        if( writelen > 0 ) {

        } else if(writelen == 0) {
                HalPanic("Wrote 0 to STDOUT");
        } else {
                HalPanicErrno("Failed to write to STDOUT");
        }
}

void * HalMap(char * tag, void * addr, COUNT len)
{
	void * ptr;
	int status;
	int fd = open(tag, O_CREAT|O_RDWR, 0666);
	if (fd == -1) {
		HalPanicErrno("Failed to open file");
	}
	status = ftruncate(fd, len);
	if (status != 0) {
		HalPanicErrno("Failed to truncate file");
	}
	ptr = mmap(
			addr,
			len,
			PROT_READ|PROT_WRITE|PROT_EXEC,
			MAP_FILE|MAP_SHARED, // MAP_HASSEMAPHORE?
			fd,
			0
		  );
	if (ptr == MAP_FAILED) {
		HalPanicErrno("Failed to create mapping");
	}

	status = close(fd);
	if (status != 0) {
		HalPanicErrno("Failed to close file");
	}
	return ptr;
}
