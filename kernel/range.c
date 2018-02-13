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

