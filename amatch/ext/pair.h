#ifndef PAIR_H_INCLUDED
#define PAIR_H_INCLUDED

#include "ruby.h"

enum { PAIR_ACTIVE = 0, PAIR_INACTIVE, PAIR_END };

typedef struct PairStruct {
    char fst;
    char snd;
    char status;
    char __align;
} Pair;

Pair *PairArray_new(VALUE tokens);

#endif
  /* vim: set et cindent sw=4 ts=4: */ 
