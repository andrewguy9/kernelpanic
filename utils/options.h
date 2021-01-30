#ifndef _OPTIONS_H
#define _OPTIONS_H

#ifdef _FEATURES_H
#error options must be included before any libc headers to select the correct options.
#endif /*_FEATURES_H*/

#ifdef LINUX
#define _GNU_SOURCE // Enable access to linux specific sigset operations.
#endif /*LINUX*/

#endif /*_OPTIONS_H*/
