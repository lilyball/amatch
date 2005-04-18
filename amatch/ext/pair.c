#include "pair.h"

Pair *PairArray_new(VALUE tokens)
{
#if 0
    int i; 
    Pair *pairs = ALLOC_N(Pair, string_len);
    MEMZERO(pairs, Pair, string_len);
    for (i = 0; i < string_len - 1; i++) {
        pairs[i].fst = string[i];
        pairs[i].snd = string[i + 1];
        pairs[i].status = PAIR_ACTIVE;
    }
    pairs[i].status = PAIR_END;
    return pairs;
#endif
}

double pair_array_match(Pair *self, Pair *other)
{
    
}
  /* vim: set et cindent sw=4 ts=4: */ 
