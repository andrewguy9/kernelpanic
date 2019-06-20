#ifndef BLOCKS_H
#define BLOCKS_H

// TODO use utils version.
#define min(x, y) ((x) > (y) ? (y) : (x))

#if defined(__clang__)
#define CAPTURE __block

#define lambda(ReturnType, arguments, ...) \
  ^ ReturnType arguments __VA_ARGS__

#define lambdaRef(RefName, ReturnType, ...) \
  ReturnType (^ RefName) (__VA_ARGS__)

#elif defined(__GNUC__) || defined(__GNUG__)

#define CAPTURE

#define lambda(l_ret_type, l_arguments, l_body)        \
  ({                                                   \
   l_ret_type l_anonymous_functions_name l_arguments   \
   l_body                                              \
   &l_anonymous_functions_name;                        \
   })

#define lambdaRef(l_name, l_ret_type, l_arguments) \
  l_ret_type (*l_name) l_arguments

#endif // Compiler check


#define forPtrs(T) \
  lambda(void, (lambdaRef(fn, void, T), T **cursor, T *stop), { \
    for(; *cursor != stop; (*cursor)+=1) { \
      fn(**cursor); \
    } \
  })

#define forEach(T) \
  lambda(void, (lambdaRef(fn, void, T), T arr[], size_t n), { \
    forPtrs(T)(fn, &arr, arr+n); \
  })

/* Maps from A -> B partially from cursor to a_stop and output to b_stop until
 * a stop is encountered. Returns the number of mapings computed. cursor and
 * output are advanced to positions after the mapping.
 * For internal library use to build more user friendly abstractions. Don't
 * use for application development.
 */
#define mapPartial(A,B) \
  lambda(void, (lambdaRef(fn, B, A), A **cursor, A *a_stop, B **output, B *b_stop), { \
    for(; *cursor != a_stop && *output != b_stop; (*cursor)++, (*output)++) { \
      **output = fn(**cursor); \
    } \
  })

#define map(A,B) \
  lambda(void, (lambdaRef(fn, B, A), A input[], B output[], size_t n), { \
    mapPartial(A,B)( fn, &input, input + n, &output, output + n); \
  })

#define filterPartial(A) \
  lambda(void, (lambdaRef(fn, _Bool, A), A **as, A *a_stop, A **os, A * o_stop), { \
    for(; *as != a_stop && *os != o_stop; (*as)++) { \
      if (fn(**as)) { \
        **os = **as; \
        (*os)++; \
      } \
    } \
  })

//TODO assumes in and out are same size.
/* 
 * Take values in arr and pass them to fn. If fn evaluates to true, add to out.
 * arr and out are allowed to be the same array.
 */
#define filter(T) \
  lambda(size_t, (lambdaRef(fn, _Bool, T), T arr[], T out[], size_t n), { \
    T * data = &out[0]; \
    filterPartial(T)(fn, &arr, &arr[n], &data, &out[n]); \
    return data - out; \
  })

#define reduce(A, B) \
  lambda(B, (lambdaRef(fn, B, B, A), A arr[], size_t n, B start), { \
    B accum = start; \
    for(size_t i=0; i<n; i++) { \
      accum = fn(accum, arr[i]); \
    } \
    return accum; \
  })

#endif // BLOCKS_H
