#include"utils/utils.h"

#include"kernel/context.h"
#include"kernel/coroutine.h"
#include"kernel/panic.h"

STACK_INIT_ROUTINE CoRoutineWrapper;
void CoRoutineWrapper (void * arg) {
  struct COROUTINE * coroutine = arg;
  coroutine->Foo(coroutine->Params, coroutine->Result, &coroutine->YieldContext);
  // Foo has returned, from now on, return that we are done.
  while (true) {
    //tell them to not come back, jump to caller.
    coroutine->YieldContext.Status = COROUTINE_DONE;
    ContextSwitch(&coroutine->YieldContext.RoutineState, &coroutine->YieldContext.CallerState);
  }
  KernelPanic();
}

void CoroutineInit(
    COROUTINE_FUNCTION foo,
    void * params,
    void * output,
    void * stack,
    COUNT size,
    struct COROUTINE * coroutine) {
  struct COROUTINE_CONTEXT *yield;

  coroutine->Foo = foo;
  coroutine->Params = params;
  coroutine->Result = output;
  yield = &coroutine->YieldContext;
  //Dont initialize yield->Status, because its done by next.
  //Dont initialize coroutine->YieldContext.CallerState, because its done when switching away from function.
  ContextInit(
      &yield->RoutineState,
      stack,
      size,
      CoRoutineWrapper,
      coroutine);
}

void CoroutineYield(struct COROUTINE_CONTEXT * yield) {
  yield->Status = COROUTINE_MORE;
  ContextSwitch(&yield->RoutineState, &yield->CallerState);
}

enum COROUTINE_STATUS CoroutineNext(struct COROUTINE * routine) {
  ContextSwitch(&routine->YieldContext.CallerState, &routine->YieldContext.RoutineState);
  // The co-routine has woken us up.
  // Lets return the results.
  return routine->YieldContext.Status;
}
