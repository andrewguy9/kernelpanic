#include "buffer.h"
#include "strings.h"
#include <stdio.h>

typedef DATA(char) chardata_t;
typedef SPACE(char) charspace_t;

typedef DATA(int) intdata_t;
typedef SPACE(int) intspace_t;

chardata_t buffer_gets(charspace_t *space) {
  char *str = fgets(space->start, space->end - space->start, stdin);
  if (str == NULL) {
    return (chardata_t){NULL, NULL};
  } else {
    size_t len = strlen(str);
    space->start += len;
    return (chardata_t){str, str + len};
  }
}

char global[10];
charspace_t space = FROM_ARRAY(global);

int main() {
  chardata_t data;
  do {
    data = buffer_gets(&space);
    if (EMPTY(chardata_t)(data)) {
      printf("was empty\n");
    } else {
      char local[10];
      charspace_t space2 = FROM_ARRAY(local);
      chardata_t copy = COPY(chardata_t, charspace_t)(data, &space2);
      printf("copy:");
      BufferForEach(char, chardata_t)(
          lambda(void, char x) { printf("%c ", x); }, copy);
      printf("\n");

      int inced[10];
      intspace_t inced_space = FROM_ARRAY(inced);
      intdata_t inced_data =
          BufferMap(char, int, chardata_t, intspace_t, intdata_t)(
              lambda(int, char x) { return x + 1; }, copy, &inced_space);
      printf("ints:");
      BufferForEach(int, intdata_t)(
          lambda(void, int x) { printf("%d ", x); }, inced_data);
      printf("\n");

      char digits[10];
      charspace_t digit_space = FROM_ARRAY(digits);
      chardata_t digits_data = BufferFilter(char, chardata_t, charspace_t)(
          lambda(_Bool, char x) { return x > '0' && x < '9'; }, data,
          &digit_space);
      printf("digits:");
      BufferForEach(char, chardata_t)(
          lambda(void, char x) { printf("%c ", x); }, digits_data);
      printf("\n");
      int sum = BufferReduce(char, int, chardata_t)(
          lambda(int, int acc, char y) { return acc + y - '0'; }, digits_data,
          0);
      printf("digits sum is %d\n", sum);
    }
  } while (!EMPTY(chardata_t)(data));
  return 0;
}
