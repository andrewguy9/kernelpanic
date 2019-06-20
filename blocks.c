#include<stdio.h>
#include<stdlib.h>

#include"blocks.h"

void test_forPtrs() {
  printf("testing forPtrs...\n");
  int data[] = {1,2,3};
  CAPTURE int count = 0;
  CAPTURE int sum = 0;
  int *cursor = &data[0];
  int *end = &data[3];
  forPtrs(int)(lambda(void, (int x), {count+=1; sum+=x;}), &cursor, end);
  printf("count,sum [1,2,3] = %d, %d\n", count, sum);
}

void test_forEach() {
  printf("testing forEach...\n");
  int data[] = {1,2,3};
  CAPTURE int count = 0;
  CAPTURE int sum = 0;
  forEach(int)(lambda(void, (int x), {count+=1; sum+=x;}), data, 3);
  printf("count,sum [1,2,3] = %d, %d\n", count, sum);
}

void test_lamdaRef() {
  printf("testing lambdaRef...\n");
  lambdaRef(fn, int, int, int) = lambda(int, (int x, int y), {return x+y;});
  printf("+ of 1,2 = %d\n", fn(1,2));
}

void test_mapPartial(char * caseName, int *data_start, int *data_end, int *space_start, int *space_end) {
  int *data_cursor = data_start, *space_cursor = space_start;
  printf("case %s: [", caseName);
  for(int * cur = data_start; cur != data_end; cur++) {
    printf("%d,", *cur);
  }
  printf("] -> [");
  mapPartial(int, int)(
      lambda(int, (int x), {return x+1;}), &data_cursor, data_end, &space_cursor, space_end);
  int mapped = data_cursor - data_start;
  for(int i=0; i<mapped; i++) {
    printf("%d,", space_start[i]);
  }
  printf("]\n");
}

void test_mapPartials() {
  printf("testing mapPartial...\n");
  {
    int arr1[0];
    int arr2[0];
    test_mapPartial("empty 2 arrays", &arr1[0], &arr1[0], &arr2[0], &arr2[0]);
  }
  {
    int arr[0];
    test_mapPartial("empty 1 array", &arr[0], &arr[0], &arr[0], &arr[0]);
  }
  {
    int arr1[4] = {1,2,3,4};
    int arr2[4];
    test_mapPartial("equal 2 arrays", &arr1[0], &arr1[4], &arr2[0], &arr2[4]);
  }
  {
    int arr1[4] = {1,2,3,4};
    test_mapPartial("equal 1 array", &arr1[0], &arr1[4], &arr1[0], &arr1[4]);
  }
  {
    int arr1[4] = {1,2,3,4};
    int arr2[2];
    test_mapPartial("short results, 2 array", &arr1[0], &arr1[4], &arr2[0], &arr2[2]);
  }
  {
    int arr1[2] = {1,2};
    int arr2[4];
    test_mapPartial("short inputs, 2 array", &arr1[0], &arr1[2], &arr2[0], &arr2[4]);
  }
  {
    int arr1[4] = {1,2,3,4};
    test_mapPartial("short results, 1 array", &arr1[0], &arr1[4], &arr1[0], &arr1[2]);
  }
  {
    int arr1[4] = {1,2,3,4};
    test_mapPartial("short inputs, 1 array", &arr1[0], &arr1[2], &arr1[0], &arr1[4]);
  }
}

void test_map(char * caseName, int data[], int space[], size_t n) {
  printf("case %s: [", caseName);
  for(int i = 0; i<n; i++) {
    printf("%d,", data[i]);
  }
  printf("] -> [");
  map(int, int)(
      lambda(int, (int x), {return x+1;}), data, space, n);
  for(int i=0; i<n; i++) {
    printf("%d,", space[i]);
  }
  printf("]\n");
}

void test_maps() {
  printf("Testing map...\n");
  {
    int arr1[0];
    int arr2[0];
    test_map("empty 2 arrays", arr1, arr2, 0);
  }
  {
    int arr[0];
    test_map("empty 1 array", arr, arr, 0);
  }
  {
    int arr1[4] = {1,2,3,4};
    int arr2[4];
    test_map("equal 2 array", arr1, arr2, 4);
  }
  {
    int arr[4] = {1,2,3,4};
    test_map("equal 1 array", arr, arr, 4);
  }
}

void test_filterPartial(char * caseName, int *data_start, int *data_end, int *space_start, int *space_end) {
  int *data_cursor = data_start, *space_cursor = space_start;
  printf("case %s: [", caseName);
  for(int * cur = data_start; cur != data_end; cur++) {
    printf("%d,", *cur);
  }
  printf("] -> [");
  filterPartial(int)(
      lambda(_Bool, (int x), {return x % 2 == 0;}), &data_cursor, data_end, &space_cursor, space_end);
  int mapped = space_cursor - space_start;
  for(int i=0; i<mapped; i++) {
    printf("%d,", space_start[i]);
  }
  printf("]\n");
}

void test_filterPartials() {
  printf("Testing filterPartial...\n");
  {
    int arr1[0];
    int arr2[0];
    test_filterPartial("empty 2 arrays", &arr1[0], &arr1[0], &arr2[0], &arr2[0]);
  }
  {
    int arr[0];
    test_filterPartial("empty 1 array", &arr[0], &arr[0], &arr[0], &arr[0]);
  }
  {
    int arr1[4] = {1,2,3,4};
    int arr2[4];
    test_filterPartial("equal 2 arrays", &arr1[0], &arr1[4], &arr2[0], &arr2[4]);
  }
  {
    int arr1[4] = {1,2,3,4};
    test_filterPartial("equal 1 array", &arr1[0], &arr1[4], &arr1[0], &arr1[4]);
  }
  {
    int arr1[6] = {1,2,3,4,5,6};
    int arr2[2];
    test_filterPartial("short results, 2 array", &arr1[0], &arr1[6], &arr2[0], &arr2[2]);
  }
  {
    int arr1[2] = {1,2};
    int arr2[4];
    test_filterPartial("short inputs, 2 array", &arr1[0], &arr1[2], &arr2[0], &arr2[4]);
  }
  {
    int arr1[6] = {1,2,3,4,5,6};
    test_filterPartial("short results, 1 array", &arr1[0], &arr1[6], &arr1[0], &arr1[2]);
  }
  {
    int arr1[4] = {1,2,3,4};
    test_filterPartial("short inputs, 1 array", &arr1[0], &arr1[2], &arr1[0], &arr1[4]);
  }
}

void test_filter(char * caseName, int data[], int space[], int n) {
  printf("case %s: [", caseName);
  for(int i = 0; i < n; i++) {
    printf("%d,", data[i]);
  }
  printf("] -> [");
  int mapped = filter(int)(
      lambda(_Bool, (int x), {return x % 2 == 0;}), data, space, n);
  for(int i=0; i<mapped; i++) {
    printf("%d,", space[i]);
  }
  printf("]\n");
}

void test_filters() {
  printf("Testing filter...\n");
  {
    int arr1[0];
    int arr2[0];
    test_filter("empty 2 arrays", arr1, arr2, 0);
  }
  {
    int arr[0];
    test_filter("empty 1 array", arr, arr, 0);
  }
  {
    int arr1[4] = {1,2,3,4};
    int arr2[4];
    test_filter("2 arrays", arr1, arr2, 4);
  }
  {
    int arr1[4] = {1,2,3,4};
    test_filter("1 array", arr1, arr1, 4);
  }
}

void test_reduce(char * caseName, int data[], int n) {
  printf("case %s: [", caseName);
  for(int i = 0; i < n; i++) {
    printf("%d,", data[i]);
  }
  int reduced = reduce(int, int)(
      lambda(int, (int acc, int x), {return acc + x;}), data, n, 0);
  printf("] -> %d\n", reduced);
}

void test_reduces() {
  printf("Testing reduce...\n");
  {
    int arr[0];
    test_reduce("empty", arr, 0);
  }
  {
    int arr[4] = {1,2,3,4};
    test_reduce("array", arr, 4);
  }
}

int main() {
  test_forPtrs();
  test_forEach();
  test_lamdaRef();
  test_mapPartials();
  test_maps();
  test_filterPartials();
  test_filters();
  test_reduces();
}
