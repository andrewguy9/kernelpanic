#ifndef RANGE_H
#define RANGE_H

#include"kernel/hal.h"
#include"kernel/context.h"

struct RANGE {
  INDEX Low;
  INDEX High;
  COUNT Step;
  INDEX Last;
};

enum CO_ROUTINE_STATUS {
  CO_ROUTINE_MORE,
  CO_ROUTINE_DONE
};

struct RANGE_RESULT {
  enum CO_ROUTINE_STATUS State;
  INDEX Last;
};

struct RANGE_COROUTINE {
  struct CONTEXT CallerState;
  struct CONTEXT RoutineState;
  struct RANGE_RESULT Result;
  struct RANGE Params;
  char Stack[HAL_MIN_STACK_SIZE];
};

void RangeRoutineInit(INDEX low, INDEX high, COUNT step, struct RANGE_COROUTINE * range);
struct RANGE_RESULT RangeRoutineNext(struct RANGE_COROUTINE * range);

#endif // RANGE_H
