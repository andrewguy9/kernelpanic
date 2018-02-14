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

struct RANGE_COROUTINE * RangeBootstrapGlobal; //TOD CAN WE GET RID OF THIS?
STACK_INIT_ROUTINE RangeRoutineInner;
void RangeRoutineInner() {
  struct RANGE_COROUTINE * range = RangeBootstrapGlobal;
  int status;
  INDEX cur = range->Params.Low;
  status = _setjmp(range->RoutineState.Registers);
  if (status == 0) {
    // We have saved initial conditions for first call to next.
    // Return to the caller.
    _longjmp(range->CallerState.Registers, 1);
  }
  // We have woken up due to a call to next.
  while (cur < range->Params.High) {
    //Copy the last iteration to output.
    range->Result.State = RANGE_MORE;
    range->Result.Last = cur;
    //Save current routine state.
    status = _setjmp(range->RoutineState.Registers);
    if (status == 0) {
      // We have done work, and copied it to output.
      // Return control to the caller.
      _longjmp(range->CallerState.Registers, 1);
    }
    // The co-routine was invoked via next.
    // Lets do more work.
    cur += range->Params.Step;
  }
  // We have hit exit condition.
  // Save that we got here, in case he checks muliple times.
  status = _setjmp(range->RoutineState.Registers);
  //TODO CHECK STATUS FOR ERRORS.
  //tell them to not come back, jump to caller.
  range->Result.State = RANGE_DONE;
  _longjmp(range->CallerState.Registers, 1);
  KernelPanic();
}

void RangeRoutineInit(INDEX low, INDEX high, COUNT step, struct RANGE_COROUTINE * range) {
  range->Params.Low = low;
  range->Params.High = high;
  range->Params.Step = step;
  range->Params.Last = low;
  RangeBootstrapGlobal = range; //TODO THIS ISN'T PROTECTED.
  ContextInit(&range->RoutineState, range->Stack, HAL_MIN_STACK_SIZE, RangeRoutineInner);

}

struct RANGE_RESULT RangeRoutineNext(struct RANGE_COROUTINE * range) {
  int status;
  status = _setjmp(range->CallerState.Registers);
  if (status == 0) {
    // We just saved our context, so that we can call the co-routine.
    // Call it via jump.
    _longjmp(range->RoutineState.Registers, 1);
  }
  // The co-routine has woken us up.
  // Lets return the results.
  return range->Result;
}
