#include"utils/utils.h"

#include"kernel/context.h"
#include"kernel/range.h"
#include"kernel/panic.h"

void Range_Init(INDEX low, INDEX high, COUNT step, struct RANGE * range) {
  range->Low = low;
  range->High = high;
  range->Step = step;
  range->Last = low;
}

enum RANGE_STATUS RangeNext(struct RANGE * range, INDEX * output) {
  range->Last = range->Last + range->Step;
  if (range->Last > range->High) {
    return RANGE_DONE;
  } else {
    * output = range->Last;
    return RANGE_MORE;
  }
}

struct RANGE_RESULT RangeGlobal(BOOL reset, INDEX low, INDEX high, COUNT step) {
  struct RANGE_RESULT result;
  static struct RANGE range;

  if (reset) {
    Range_Init(low, high, step, &range);
    result.State = RANGE_MORE;
    result.Last = low;
    return result;
  }

  result.State = RangeNext(&range, &result.Last);
  return result;
}

STACK_INIT_ROUTINE RangeRoutineInner;
void RangeRoutineInner(void * arg) {
  struct RANGE_COROUTINE * range = arg;
  INDEX cur = range->Params.Low;
  // We have woken up due to a call to next.
  while (cur < range->Params.High) {
    //Copy the last iteration to output.
    range->Result.State = RANGE_MORE;
    range->Result.Last = cur;
    //Save current routine state.
    ContextSwitch(&range->RoutineState, &range->CallerState);
    // The co-routine was invoked via next.
    // Lets do more work.
    cur += range->Params.Step;
  }
  // We have hit exit condition.
  while (TRUE) {
    //tell them to not come back, jump to caller.
    range->Result.State = RANGE_DONE;
    ContextSwitch(&range->RoutineState, &range->CallerState);
  }
  KernelPanic();
}

void RangeRoutineInit(INDEX low, INDEX high, COUNT step, struct RANGE_COROUTINE * range) {
  range->Params.Low = low;
  range->Params.High = high;
  range->Params.Step = step;
  range->Params.Last = low;
  ContextInit(&range->RoutineState, range->Stack, HAL_MIN_STACK_SIZE, RangeRoutineInner, range);

}

struct RANGE_RESULT RangeRoutineNext(struct RANGE_COROUTINE * range) {
  ContextSwitch(&range->CallerState, &range->RoutineState);
  // The co-routine has woken us up.
  // Lets return the results.
  return range->Result;
}
