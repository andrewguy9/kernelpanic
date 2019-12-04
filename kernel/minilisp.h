#ifndef MINILISP_H
#define MINILISP_H

#include"kernel/scheduler.h"
#include "utils/utils.h"

// Typedef for the primitive function
struct Obj;
typedef struct Obj *Primitive(void *root, struct Obj **env, struct Obj **args);

// The object type
typedef struct Obj {
  // The first word of the object represents the type of the object. Any code that handles object
  // needs to check its type first, then access the following union members.
  int type;

  // The total size of the object, including "type" field, this field, the contents, and the
  // padding at the end of the object.
  int size;

  // Object values.
  union {
    // Int
    int value;
    // Cell
    struct {
      struct Obj *car;
      struct Obj *cdr;
    };
    // Symbol
    char name[1];
    // Primitive
    Primitive *fn;
    // Function or Macro
    struct {
      struct Obj *params;
      struct Obj *body;
      struct Obj *env;
    };
    // Environment frame. This is a linked list of association lists
    // containing the mapping from symbols to their value.
    struct {
      struct Obj *vars;
      struct Obj *up;
    };
    // Forwarding pointer
    void *moved;
  };
} Obj;

struct ALLOC_BLOCK {
  // The pointer pointing to the beginning of the current heap
  void *memory;

  // The pointer pointing to the beginning of the old heap
  void *from_space;

  // The number of bytes allocated from the heap
  size_t mem_nused;

  void *cur_heap;
  void *next_heap;

  size_t memory_size;

  // Flags to debug GC
  _Bool gc_running;
  _Bool debug_gc;
  _Bool always_gc;

  // The list containing all symbols. Such data structure is traditionally called the "obarray", but I
  // avoid using it as a variable name as this is not an array but a list.
  Obj *Symbols;

};

void lisp_init(
    struct ALLOC_BLOCK * block,
    void * heap1,
    void * heap2,
    size_t heap_size,
    _Bool debug_gc,
    _Bool always_gc);

THREAD_MAIN lisp_repl_main;

#endif
