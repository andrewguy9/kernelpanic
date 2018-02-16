#ifndef RANGE_H
#define RANGE_H

#include"kernel/hal.h"
#include"kernel/context.h"

enum CO_ROUTINE_STATUS {
  CO_ROUTINE_MORE,
  CO_ROUTINE_DONE
};

struct CO_ROUTINE_CONTEXT {
  enum CO_ROUTINE_STATUS Status;
  struct CONTEXT CallerState;
  struct CONTEXT RoutineState;
};

typedef void (CO_ROUTINE_FUNCTION) (void * params, void * result, struct CO_ROUTINE_CONTEXT * yield);

struct CO_ROUTINE {
  CO_ROUTINE_FUNCTION * Foo;
  void * Params;
  void * Result;
  struct CO_ROUTINE_CONTEXT YieldContext;
  char Stack[HAL_MIN_STACK_SIZE];
};

void CoroutineInit(
    CO_ROUTINE_FUNCTION foo,
    void * params,
    void * output,
    struct CO_ROUTINE * coroutine);

void CoroutineYield(struct CO_ROUTINE_CONTEXT * yield);

enum CO_ROUTINE_STATUS CoroutineNext(struct CO_ROUTINE * routine);

#endif // RANGE_H
