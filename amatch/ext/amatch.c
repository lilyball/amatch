#include "ruby.h"

#define LEVENSHTEIN_EACH(mode)                                              \
int i;                                                                      \
VALUE result;                                                               \
VALUE pattern = rb_iv_get(self, "@pattern");                                \
Check_Type(pattern, T_STRING);                                              \
result = rb_ary_new2(argc);                                                 \
if (argc == 0)                                                              \
    rb_raise(rb_eArgError, "wrong number of arguments (%d for 1)", argc);   \
if (argc == 1)                                                              \
    return compute_levenshtein_distance(self, pattern, argv[0], mode);      \
for (i = 0; i < argc; i++) {                                                \
    if (TYPE(argv[i]) != T_STRING) {                                        \
        rb_raise(rb_eTypeError,                                             \
            "argument #%d has to be a string (%s given)", i + 1,            \
            NIL_P(argv[i]) ? "NilClass" : rb_class2name(CLASS_OF(argv[i])));\
    }                                                                       \
    rb_ary_push(result,                                                     \
        compute_levenshtein_distance(self, pattern, argv[i], mode));        \
}                                                                           \
return result;

static VALUE cAmatch;

/*
 * Vector stuff
 */

typedef struct {
    int *ptr;
    int len;
} vector;

static vector *
vector_new(len)
    int len;
{
    vector *v;
    v = ALLOC(vector);
    if (v == NULL) rb_raise(rb_eNoMemError, "couldn't malloc vector");
    v->ptr = ALLOC_N(int, len + 1);
    if (v->ptr == NULL) rb_raise(rb_eNoMemError, "couldn't malloc vector data");
    v->len = len;
    return v;
}

static void
vector_print(v)
    vector *v;
{
    int i;
    for(i = 0; i < v->len; i++) printf("%d", v->ptr[i]);
    puts("");
}

static void
vector_destroy(v)
    vector *v;
{
    xfree(v->ptr);
    xfree(v);
}

static int
vector_minimum(v)
    vector *v;
{
    int i;
    int min;

    if (v->len == 0) return -1;
    min = v->ptr[0];
    for (i = 1; i <= v->len; i++) {
        if (min > v->ptr[i]) min = v->ptr[i];
    }
    return min;
}

static int
vector_last(v)
    vector *v;
{
    return v->ptr[v->len];
}

/*
 * Levenshtein edit distances are calculated here:
 */

enum { MATCH = 1, MATCHR, SEARCH, SEARCHR, COMPARE, COMPARER };

static int weight2int(weight, name)
    VALUE weight;
    char *name;
{
    if (TYPE(weight) != T_FIXNUM) {
        rb_raise(rb_eTypeError,
            "value of weight %s has to be of type Fixnum (%s given)",
            "subw", NIL_P(weight) ? "NilClass" : rb_class2name(CLASS_OF(weight)));
    }
    return FIX2INT(weight);
}

static VALUE
compute_levenshtein_distance(self, pattern, string, mode)
    VALUE self;
    VALUE pattern;
    VALUE string;
    char mode;
{
    static VALUE result;
    int pattern_len, string_len;
    char *pattern_ptr, *string_ptr;
    vector *v[2];
    int weight, sw, dw, iw, i, j, tmpi;
    int c = 0, p = 1;

    Check_Type(string, T_STRING);
    string_ptr = RSTRING(string)->ptr;
    string_len = RSTRING(string)->len;

    pattern_ptr = RSTRING(pattern)->ptr;
    pattern_len = RSTRING(pattern)->len;

    sw = weight2int(rb_iv_get(self, "@subw"), "subw");
    dw = weight2int(rb_iv_get(self, "@delw"), "delw");
    iw = weight2int(rb_iv_get(self, "@insw"), "insw");
    
    v[0] = vector_new(string_len);
    switch (mode) {
        case MATCH:
        case MATCHR:
        case COMPARE:
        case COMPARER:
            for (i = 0; i <= v[0]->len; i++) v[0]->ptr[i] = i * iw;
            break;
        case SEARCH:
        case SEARCHR:
            for (i = 0; i <= v[0]->len; i++) v[0]->ptr[i] = 0;
            break;
        default:
            rb_raise(rb_eFatal, "unknown mode in compute_levenshtein_distance");
    }

    v[1] = vector_new(string_len);
    for (i = 1; i <= pattern_len; i++) {
        c = i % 2;                /* current row */
        p = (i - 1) % 2;          /* previous row */
        v[c]->ptr[0] = i * dw;    /* first column */
        for (j = 1; j <= string_len; j++) {
            /* Bellman's principle of optimality: */
            weight = v[p]->ptr[j - 1] +
                (pattern_ptr[i - 1] == string_ptr[j - 1] ? 0 : sw);
             if (weight > v[p]->ptr[j] + 1) weight = v[p]->ptr[j] + dw;
            if (weight > v[c]->ptr[j - 1] + 1) weight = v[c]->ptr[j - 1] + iw;
            v[c]->ptr[j] = weight;
        }
    }
    switch (mode) {
        case MATCH:
            result = INT2FIX(vector_last(v[c]));
            break;
        case MATCHR:
            result = rb_float_new((double) vector_last(v[c]) / pattern_len);
            break;
        case SEARCH:
            tmpi = vector_minimum(v[c]);
            result = tmpi < 0 ? INT2FIX(pattern_len) : INT2FIX(tmpi);
            break;
        case SEARCHR:
            tmpi = vector_minimum(v[c]);
            result = rb_float_new( tmpi < 0 ? 1.0 : (double) tmpi / pattern_len);
            break;
        case COMPARE:
            result = INT2FIX((string_len < pattern_len ? -1 : 1) *
                vector_last(v[c]));
            break;
        case COMPARER:
            result = rb_float_new((double)
                (string_len < pattern_len ? -1 : 1)     *
                vector_last(v[c]) / pattern_len);
            break;
        default:
            rb_raise(rb_eFatal, "unknown mode in compute_levenshtein_distance");
    }
    vector_destroy(v[0]);
    vector_destroy(v[1]);
    return result;
}

/*
 * Ruby API
 */

static VALUE
rb_amatch_resetw(self)
    VALUE self;
{
    rb_iv_set(self, "@subw", INT2FIX(1));
    rb_iv_set(self, "@delw", INT2FIX(1));
    rb_iv_set(self, "@insw", INT2FIX(1));

    return Qtrue;
}

static VALUE
rb_amatch_initialize(self, pattern)
    VALUE self;
    VALUE pattern;
{

    Check_Type(pattern, T_STRING);
    rb_iv_set(self, "@pattern", pattern);
    rb_amatch_resetw(self);
    return self;
}

static VALUE
rb_amatch_match(argc, argv, self)
    int argc;
    VALUE *argv;
    VALUE self;
{
    LEVENSHTEIN_EACH(MATCH);
}

static VALUE
rb_amatch_matchr(argc, argv, self)
    int argc;
    VALUE *argv;
    VALUE self;
{
    LEVENSHTEIN_EACH(MATCHR);
}

static VALUE
rb_amatch_compare(argc, argv, self)
    int argc;
    VALUE *argv;
    VALUE self;
{
    LEVENSHTEIN_EACH(COMPARE);
}

static VALUE
rb_amatch_comparer(argc, argv, self)
    int argc;
    VALUE *argv;
    VALUE self;
{
    LEVENSHTEIN_EACH(COMPARER);
}


static VALUE
rb_amatch_search(argc, argv, self)
    int argc;
    VALUE *argv;
    VALUE self;
{
    LEVENSHTEIN_EACH(SEARCH);
}

static VALUE
rb_amatch_searchr(argc, argv, self)
    int argc;
    VALUE *argv;
    VALUE self;
{
    LEVENSHTEIN_EACH(SEARCHR);
}

static VALUE
rb_amatch_pair_distance(argc, argv, self)
    int argc;
    VALUE *argv;
    VALUE self;
{
    return Qnil;
}

void
Init_amatch()
{
    cAmatch = rb_define_class("Amatch", rb_cObject);
    rb_define_method(cAmatch, "initialize", rb_amatch_initialize, 1);

    rb_define_attr(cAmatch, "debug", 1, 1);
    rb_define_attr(cAmatch, "subw", 1, 1);
    rb_define_attr(cAmatch, "delw", 1, 1);
    rb_define_attr(cAmatch, "insw", 1, 1);
    rb_define_method(cAmatch, "resetw", rb_amatch_resetw, 0);

    rb_define_attr(cAmatch, "pattern", 1, 1);

    rb_define_method(cAmatch, "match", rb_amatch_match, -1);
    rb_define_method(cAmatch, "matchr", rb_amatch_matchr, -1);
    rb_define_method(cAmatch, "compare", rb_amatch_compare, -1);
    rb_define_method(cAmatch, "comparer", rb_amatch_comparer, -1);
    rb_define_method(cAmatch, "search", rb_amatch_search, -1);
    rb_define_method(cAmatch, "searchr", rb_amatch_searchr, -1);
    rb_define_method(cAmatch, "pair_distance", rb_amatch_pair_distance, -1);
}
    /* vim: set et cin sw=4 ts=4: */
