#ifndef UTILS_H
#define UTILS_H

#ifndef NULL
#define NULL 0
#endif

#define FALSE 0
#define TRUE (! FALSE )

#include<stdio.h>
#include<stdlib.h>
#define ASSERT( condition, error ) ( condition ? :  \
		printf("ASSERT FAILED: %s\n", error))

typedef unsigned int COUNT;
typedef unsigned int INDEX;
typedef unsigned char BOOL;

#endif
