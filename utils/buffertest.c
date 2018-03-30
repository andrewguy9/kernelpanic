#include"buffer.h"
#include"types.h"
#include"utils.h"
#include<stdio.h>
#include<string.h>

struct TEST_STRUCT {
  COUNT Val1;
  COUNT Val2;
};

#define STRUCT_SIZE (sizeof(struct TEST_STRUCT))

void test_BufferSpace() {
  char buff[STRUCT_SIZE];
  SPACE s = BufferSpace(buff, sizeof(buff));

  ASSERT (s.Length == sizeof(buff));
  ASSERT (s.Buff = buff);
}

void test_BufferFromObj() {
  struct TEST_STRUCT obj1 = {5, 10};
  const DATA d1 = BufferFromObj(obj1);
  ASSERT(d1.Buff == (char*) &obj1);
  ASSERT(d1.Length == sizeof(d1));
}

void test_BufferData() {
  char buff[5];
  SPACE space = BufferFromObj(buff);
  BufferPrint(&space, "test");
  DATA data = BufferData(buff, &space);
  ASSERT (data.Buff == buff);
  ASSERT (data.Length = 5);
  ASSERT (strcmp("test", data.Buff) == 0);
}

void test_BufferToObj() {
  struct TEST_STRUCT obj1 = {5, 10};
  DATA data = BufferFromObj(obj1);
  struct TEST_STRUCT obj2 = BufferToObj(data, struct TEST_STRUCT);
  ASSERT(obj2.Val1 == 5);
  ASSERT(obj2.Val2 == 10);
}

void test_BufferCopy() {
  struct TEST_STRUCT obj = {5,10};
  DATA inData = BufferFromObj(obj);
  char buff[sizeof(obj)];
  SPACE space = BufferFromObj(buff);
  BufferCopy(&inData, &space);
  ASSERT(BufferEmpty(&inData));
  ASSERT(BufferEmpty(&space));
  DATA out_data = BufferData(buff, &space);
  struct TEST_STRUCT copy = {0,0};
  copy = BufferToObj(out_data, struct TEST_STRUCT);
  ASSERT (copy.Val1 = 5);
  ASSERT (copy.Val2 = 10);
}

void test_struct()
{
  test_BufferSpace();
  test_BufferFromObj();
  test_BufferData();
  test_BufferToObj();
  test_BufferCopy();
}

void test_string()
{
  char b1[4];
  SPACE s1 = BufferSpace(b1, sizeof(b1));
  ASSERT (BufferPrint(&s1, "%d", 123));
  ASSERT (strcmp("123", b1) == 0);
  ASSERT (s1.Buff == b1 + 3);
  ASSERT (s1.Length == 1);

  char b2[4];
  SPACE s2 = BufferSpace(b2, sizeof(b2));
  ASSERT (!BufferPrint(&s2, "%d", 1234));
  ASSERT (strcmp("123", b2) == 0);
  ASSERT (s2.Buff == b2 + sizeof(b2));
  ASSERT (BufferFull(&s2));

  char b3[8];
  SPACE s3 = BufferSpace(b3, sizeof(b3));
  ASSERT (BufferPrint(&s3, "%d", 123));
  ASSERT (strcmp("123", b3) == 0);
  ASSERT (s3.Length == sizeof(b3) - 3);

  ASSERT (BufferPrint(&s3, "%d", 987));
  ASSERT (s3.Length == sizeof(b3) - 6);
  ASSERT (strcmp("123987", b3) == 0);

  ASSERT (!BufferPrint(&s3, "%d", 456));
  ASSERT (BufferFull(&s3));
  ASSERT (strcmp("1239874", b3) == 0);
}

int main() {
  test_struct();
  test_string();
  return 0;
}
