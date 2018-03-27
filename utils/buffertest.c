#include"buffer.h"
#include"types.h"
#include"utils.h"
#include<stdio.h>

struct TEST_STRUCT {
  COUNT Val1;
  BOOL Val2;
};

#define STRUCT_SIZE (sizeof(struct TEST_STRUCT))

void test_struct()
{
  char buff[STRUCT_SIZE];
  SPACE s = BufferSpaceInit(buff, sizeof(buff));

  ASSERT (s.Length == STRUCT_SIZE);
  ASSERT (s.Buff = buff);

  struct TEST_STRUCT s1 = {5, TRUE};
  DATA d1 = BufferDataInit(s1);
  ASSERT (BufferCopy(&d1, &s));
  ASSERT (s.Length == sizeof(buff) - sizeof(s1));
  ASSERT (s.Buff == buff + sizeof(s1));

  struct TEST_STRUCT s2 = *(struct TEST_STRUCT *) d1.Buff;
  ASSERT(s1.Val1 == s2.Val1);
  ASSERT(s1.Val2 == s2.Val2);

  struct TEST_STRUCT s3 = {10, TRUE};
  DATA d2 = BufferDataInit(s3);
  ASSERT (!BufferCopy(&d2, &s));
  ASSERT (s.Length == sizeof(buff) - sizeof(s1));
  ASSERT (s.Buff == buff + sizeof(s1));

  SPACE empty = BufferSpaceInit(NULL, 0);
  ASSERT (BufferSpaceFull(&empty));

}

#include<string.h>
void test_string()
{
  char b1[4];
  SPACE s1 = BufferSpaceInit(b1, sizeof(b1));
  ASSERT (BuffStrRender(&s1, "%d", 123));
  ASSERT (strcmp("123", b1) == 0);
  ASSERT (s1.Buff == b1 + 3);
  ASSERT (s1.Length == 1);

  char b2[4];
  SPACE s2 = BufferSpaceInit(b2, sizeof(b2));
  ASSERT (!BuffStrRender(&s2, "%d", 1234));
  ASSERT (strcmp("123", b2) == 0);
  ASSERT (s2.Buff == b2 + sizeof(b2));
  ASSERT (BufferSpaceFull(&s2));

  char b3[8];
  SPACE s3 = BufferSpaceInit(b3, sizeof(b3));
  ASSERT (BuffStrRender(&s3, "%d", 123));
  ASSERT (strcmp("123", b3) == 0);
  ASSERT (s3.Length == sizeof(b3) - 3);

  ASSERT (BuffStrRender(&s3, "%d", 987));
  ASSERT (s3.Length == sizeof(b3) - 6);
  ASSERT (strcmp("123987", b3) == 0);

  ASSERT (!BuffStrRender(&s3, "%d", 456));
  ASSERT (BufferSpaceFull(&s3));
  ASSERT (strcmp("1239874", b3) == 0);

}

int main() {
  test_struct();
  test_string();
  return 0;
}
