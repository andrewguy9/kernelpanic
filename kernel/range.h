#ifndef RANGE_H
#define RANGE_H

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

#endif // RANGE_H
