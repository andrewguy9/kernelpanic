#include"utils/utils.h"

#include"kernel/context.h"
#include"kernel/range.h"
#include"kernel/panic.h"

STACK_INIT_ROUTINE CoRoutineWrapper;
void CoRoutineWrapper (void * arg) {
  struct CO_ROUTINE * coroutine = arg;
  coroutine->Foo(coroutine->Params, coroutine->Result, &coroutine->YieldContext);
  // Foo has returned, from now on, return that we are done.
  while (TRUE) {
    //tell them to not come back, jump to caller.
    coroutine->YieldContext.Status = CO_ROUTINE_DONE;
    ContextSwitch(&coroutine->YieldContext.RoutineState, &coroutine->YieldContext.CallerState);
  }
  KernelPanic();
}

//TODO MAYBE STACK SHOULD BE EXTERNALLY SET.
void CoroutineInit(
    CO_ROUTINE_FUNCTION foo,
    void * params,
    void * output,
    struct CO_ROUTINE * coroutine) {
  coroutine->Foo = foo;
  coroutine->Params = params;
  coroutine->Result = output;
  coroutine->YieldContext.Status = CO_ROUTINE_MORE; //XXX Needed?
  //XXX didn't initialize coroutine->YieldContext.CallerState
  ContextInit(
      &coroutine->YieldContext.RoutineState,
      coroutine->Stack,
      HAL_MIN_STACK_SIZE,
      CoRoutineWrapper,
      coroutine);
}

void CoroutineYield(struct CO_ROUTINE_CONTEXT * yield) {
  yield->Status = CO_ROUTINE_MORE;
  ContextSwitch(&yield->RoutineState, &yield->CallerState);
}

enum CO_ROUTINE_STATUS CoroutineNext(struct CO_ROUTINE * routine) {
  ContextSwitch(&routine->YieldContext.CallerState, &routine->YieldContext.RoutineState);
  // The co-routine has woken us up.
  // Lets return the results.
  return routine->YieldContext.Status;
}
