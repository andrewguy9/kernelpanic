#ifndef RANGE_H
#define RANGE_H

#include"kernel/hal.h"

struct RANGE {
  INDEX Low;
  INDEX High;
  COUNT Step;
  INDEX Last;
};

enum RANGE_STATUS {
  RANGE_MORE,
  RANGE_DONE
};

void Range_Init(INDEX low, INDEX high, COUNT step, struct RANGE * range);
enum RANGE_STATUS RangeNext(struct RANGE * range, INDEX * output);

struct RANGE_RESULT {
  enum RANGE_STATUS State;
  INDEX Last;
};

struct RANGE_RESULT RangeGlobal(BOOL reset, INDEX low, INDEX high, COUNT step);

struct RANGE_COROUTINE {
  struct MACHINE_CONTEXT CallerState;
  struct MACHINE_CONTEXT RoutineState;
  struct RANGE_RESULT Result;
  // TODO We need a stack stub area to hold our frame.
};

void RangeRoutineInit(INDEX low, INDEX high, COUNT step, struct RANGE_COROUTINE * range);
struct RANGE_RESULT RangeRoutineNext(struct RANGE_COROUTINE * range);

#endif // RANGE_H
