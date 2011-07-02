#ifndef TYPES_H
#define TYPES_H

//
//TYPEDEFS
//

#ifdef AVR_BUILD
typedef unsigned int COUNT;
typedef unsigned int INDEX;
typedef unsigned char BOOL;
typedef unsigned long int TIME;
typedef unsigned char ATOMIC;
#endif

#ifdef PC_BUILD
typedef unsigned long int COUNT;
typedef unsigned long int INDEX;
typedef unsigned char BOOL;
typedef unsigned long int TIME;
typedef unsigned int ATOMIC;
#endif

#endif // TYPES_H
