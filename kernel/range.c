#include"utils/utils.h"

#include"kernel/range.h"
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

void RangeRoutineInit(INDEX low, INDEX high, COUNT step, struct RANGE_COROUTINE * range) {
  int status;

  range->Result.State = RANGE_MORE;
  range->Result.Last = low;

  while (range->Result.Last < high) {
    status = _setjmp(range->RoutineState.Registers); //save state, so we can get back
    if (status == 0) {
      // We just called setjmp.
      // Either this is a natural call to Init, or the co-routine has been invoked vai _longjmp.
      return;
    } else {
      // The co-routine was invoked via _longjmp.
      // Jump back
      _longjmp(range->CallerState.Registers, 1);
    }
    //update local counters
    range->Result.State = RANGE_MORE;
    range->Result.Last += step;
  }
  //tell them to not come back, jump to caller.
  range->Result.State = RANGE_DONE;
  _longjmp(range->RoutineState.Registers, 1);
}

struct RANGE_RESULT RangeRoutineNext(struct RANGE_COROUTINE * range) {
  int status;
  status = _setjmp(range->CallerState.Registers);
  if (status == 0) {
    //This was the saving call.
    //Lets jump to the coroutine.
    _longjmp(range->RoutineState.Registers, 1);
  } else {
    //This was the restore call from the co-routine.
    // result now has the co-routine state.
  }
  // Jump to the co-routine.
  return range->Result;
}
