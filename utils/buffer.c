#include"buffer.h"

/*
 * Test Checklist:
 * All macros tested.
 * Get typedefs for subtypes
 * Pass subtypes around.
 */

/*
 * Array tests
 */
void RunTests() {
  int a=1, b=2;
  SWAP(a,b);
  ASSERT(a==2);
  ASSERT(b==1);

  typedef int T;
  T t = 5; t = 5;

  typedef ARRAY_TYPE(T, 2) TArray;
  TArray array; ARRAY_INIT(array, 1, 2);
  ASSERT(array.Bunch[0] == 1);
  ASSERT(array.Bunch[1] == 2);
  ARRAY_INIT(array, 3, 4);
  ASSERT(array.Bunch[0] == 3);
  ASSERT(array.Bunch[1] == 4);
  _Static_assert (ARRAY_COUNT(array) == 2, "Array count check");

  typedef CURSOR_TYPE(T) TCursor;
  TCursor space; CURSOR_SPACE(space, array);
  CURSOR_SPACE(space, array);
  ASSERT ( !CURSOR_FULL(space) );
  ASSERT ( CURSOR_WRITE(t++, space));
  ASSERT ( !CURSOR_FULL(space) );
  ASSERT ( CURSOR_WRITE(t++, space));
  ASSERT (  CURSOR_FULL(space) );
  ASSERT (!CURSOR_WRITE(t++, space));
  ASSERT (  CURSOR_FULL(space) );
  TCursor data = CURSOR_DATA(TCursor, array, space);
  int * i = CURSOR_READ(data); ASSERT(*i==5);
        ASSERT ( !CURSOR_EMPTY(data) );
        i = CURSOR_READ(data); ASSERT(*i==6);
        ASSERT (  CURSOR_EMPTY(data) );
        i = CURSOR_READ(data); ASSERT(i==NULL);
        ASSERT (  CURSOR_EMPTY(data) );
  int c = 5; data = CURSOR_DATA(TCursor, array, space);
  CURSOR_FOR_EACH(data, cur) {
    ASSERT(*cur == c++);
  }
  c = 5; data = CURSOR_DATA(TCursor, array, space);
  CURSOR_FOR_EACH(data, cur) {
    ASSERT(*cur == c++);
  }
  TArray copy;
  CURSOR_SPACE(space, copy);
  data = CURSOR_DATA(TCursor, array, space);
  CURSOR_COPY(data, space);
  c = 5; data = CURSOR_DATA(TCursor, copy, space);
  CURSOR_FOR_EACH(data, cur) {
    ASSERT(*cur == c++);
  }

  typedef BUFFER_TYPE(TArray, TCursor) TBuffer;
  TBuffer bufferx; BUFFER_INIT(bufferx);
  BUFFER_INIT(bufferx);
  space = BUFFER_DATA(bufferx);
  int x=0;
  ASSERT (!BUFFER_FULL(bufferx));
  BUFFER_UNTIL_FULL(bufferx) {
    BUFFER_WRITE(x++, bufferx);
  }
  ASSERT (BUFFER_FULL(bufferx));
  TBuffer copy_buff; BUFFER_INIT(copy_buff);
  data = BUFFER_DATA(bufferx);
  BUFFER_COPY(space, copy_buff);
  BUFFER_INIT(bufferx);
  ASSERT (!BUFFER_FULL(bufferx));
  BUFFER_FINALIZE(bufferx);
  ASSERT (BUFFER_FULL(bufferx));

  typedef DOUBLE_BUFFER_TYPE(TBuffer, TCursor) TDBuffer;
  TDBuffer dbuffer = DOUBLE_BUFFER_INIT(dbuffer);
  DOUBLE_BUFFER_INIT(dbuffer);
  x = 0;
  int * y = NULL;
  DOUBLE_BUFFER_WRITE(x, dbuffer);
  y = DOUBLE_BUFFER_READ(TBuffer, TCursor, dbuffer);
  ASSERT (*y == 0);
  DOUBLE_BUFFER_FLIP(TBuffer, TCursor, dbuffer);
}

int main() {
  RunTests();
}
