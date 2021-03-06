#include"sleep.h"
#include"scheduler.h"
#include"timer.h"
#include"critinterrupt.h"
#include"signal.h"

/*
 * This function is called by the SleepTimerHandler when
 * its time to wake a thread. This function runs in
 * a critical section so it can wake a thread.
 */
HANDLER_FUNCTION SleepCritHandler;
_Bool SleepCritHandler( struct HANDLER_OBJECT * handler )
{
  struct SIGNAL * sleepSignal = handler->Context;

  ASSERT( SchedulerIsCritical() );
  SignalSet(sleepSignal);

  return true;
}

/*
 * This function is called when a thread has
 * called Sleep(), and the time he specified
 * has passed.
 *
 * We will schedule a critical handler which can wake threads.
 */
HANDLER_FUNCTION SleepTimerHandler;
_Bool SleepTimerHandler( struct HANDLER_OBJECT * timer )
{
  CritInterruptRegisterHandler(
      timer,
      SleepCritHandler,
      timer->Context );

  return false;
}

/*
 * Sleep registers a timer and then go to sleep.
 * The timer will fire at wake time.
 * The timer cannot reactivate the thread because it may fire
 * durring a thread critical section. So the timer will register
 * a critical section handler.
 *
 * In order to do this Sleep must use a HANDLER_OBJECT for
 * the timer and crit handler. I will allocate this on the stack.
 */
void Sleep( COUNT time )
{
  struct SIGNAL sleepSignal;
  struct HANDLER_OBJECT timer;

  SignalInit(&sleepSignal, false);
  HandlerInit( &timer );
  TimerRegister( &timer, time, SleepTimerHandler, &sleepSignal);
  SignalWaitForSignal(&sleepSignal, NULL);
}
