#include "buffer.h"
#include <stdio.h>
#include <string.h>

typedef DATA(char) chardata_t;
typedef SPACE(char) charspace_t;

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

int main() {
  char strMem[10];
  do {
  charspace_t strSpace = FROM_ARRAY(strMem);
  chardata_t str = buffer_gets(&strSpace);
  if (EMPTY(chardata_t)(str)) {
    break;
  }

  char filteredMem[10];
  charspace_t filteredSpace = FROM_ARRAY(filteredMem);
  chardata_t filtered = BufferFilter(char, chardata_t, charspace_t)(
      lambda(_Bool, char x) { return x > '0' && x < '9'; }, str,
      &filteredSpace);

  int sum = BufferReduce(char, int, chardata_t)(
      lambda(int, int acc, char y) { return acc + y - '0'; }, filtered, 0);
  printf("digits sum is %d\n", sum);
  } while (1);
}
