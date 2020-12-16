#ifndef DEFS_H
#define DEFS_H

#ifdef LINUX
// Enable access to linux specific sigset operations.
#define _GNU_SOURCE
#endif /* LINUX */

#endif /* DEFS_H */
