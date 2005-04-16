#ifndef VECTOR_H_INCLUDED
#define VECTOR_H_INCLUDED

#include "ruby.h"

typedef struct VectorStruct {
    int *ptr;
    int len;
} Vector;

Vector *Vector_new(int len);
void vector_print(Vector *v);
void vector_destroy(Vector *v);
int vector_minimum(Vector *v);
int vector_last(Vector *v);

#endif
  /* vim: set et cindent sw=4 ts=4: */ 
