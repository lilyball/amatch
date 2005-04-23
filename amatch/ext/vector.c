#include "vector.h"

Vector *Vector_new(int len)
{
    Vector *v = ALLOC(Vector);
    if (v == NULL) rb_raise(rb_eNoMemError, "couldn't malloc vector");
    v->ptr = ALLOC_N(double, len + 1);
    if (v->ptr == NULL) rb_raise(rb_eNoMemError, "couldn't malloc vector data");
    v->len = len;
    return v;
}

void vector_print(Vector *v)
{
    int i;
    for(i = 0; i < v->len; i++) printf("%lf", v->ptr[i]);
    puts("");
}

void vector_destroy(Vector *v)
{
    free(v->ptr);
    free(v);
}

double vector_minimum(Vector *v)
{
    int i;
    double min;

    if (v->len == 0) return -1.0;
    min = v->ptr[0];
    for (i = 1; i <= v->len; i++) {
        if (min > v->ptr[i]) min = v->ptr[i];
    }
    return min;
}

double vector_last(Vector *v) {
    return v->ptr[v->len];
}
  /* vim: set et cindent sw=4 ts=4: */ 
