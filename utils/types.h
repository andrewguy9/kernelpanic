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

#endif

#ifdef PC_BUILD
typedef unsigned long int COUNT;
typedef unsigned long int INDEX;
typedef unsigned char BOOL;
typedef unsigned long int TIME;

#endif

#endif // TYPES_H
