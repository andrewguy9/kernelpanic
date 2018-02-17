#ifndef COROUTINE_H
#define COROUTINE_H

#include"kernel/hal.h"
#include"kernel/context.h"

enum COROUTINE_STATUS {
  COROUTINE_MORE,
  COROUTINE_DONE
};

struct COROUTINE_CONTEXT {
  enum COROUTINE_STATUS Status;
  struct CONTEXT CallerState;
  struct CONTEXT RoutineState;
};

typedef void (COROUTINE_FUNCTION) (void * params, void * result, struct COROUTINE_CONTEXT * yield);

struct COROUTINE {
  COROUTINE_FUNCTION * Foo;
  void * Params;
  void * Result;
  struct COROUTINE_CONTEXT YieldContext;
};

void CoroutineInit(
    COROUTINE_FUNCTION foo,
    void * params,
    void * output,
    void * stack,
    COUNT size,
    struct COROUTINE * coroutine);

void CoroutineYield(struct COROUTINE_CONTEXT * yield);

enum COROUTINE_STATUS CoroutineNext(struct COROUTINE * routine);

#endif // COROUTINE_H
