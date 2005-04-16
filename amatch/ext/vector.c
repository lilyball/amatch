#include "vector.h"
#include "ruby.h"

Vector *Vector_new(int len)
{
    Vector *v = ALLOC(Vector);
    if (v == NULL) rb_raise(rb_eNoMemError, "couldn't malloc vector");
    v->ptr = ALLOC_N(int, len + 1);
    if (v->ptr == NULL) rb_raise(rb_eNoMemError, "couldn't malloc vector data");
    v->len = len;
    return v;
}

void vector_print(Vector *v)
{
    int i;
    for(i = 0; i < v->len; i++) printf("%d", v->ptr[i]);
    puts("");
}

void vector_destroy(Vector *v)
{
    xfree(v->ptr);
    xfree(v);
}

int vector_minimum(Vector *v) {
    int i;
    int min;

    if (v->len == 0) return -1;
    min = v->ptr[0];
    for (i = 1; i <= v->len; i++) {
        if (min > v->ptr[i]) min = v->ptr[i];
    }
    return min;
}

int vector_last(Vector *v) {
    return v->ptr[v->len];
}
