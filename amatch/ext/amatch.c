#include "ruby.h"
#include "vector.h"

/* Macromania goes here: */

#define GET_AMATCH                          \
    Amatch *amatch;                         \
    Data_Get_Struct(self, Amatch, amatch)

#define AMATCH_ACCESSOR(name) \
    rb_define_method(rb_cAmatch, #name, rb_amatch_ ## name, 0); \
    rb_define_method(rb_cAmatch, #name"=", rb_amatch_ ## name ## _set, 1)

#define DEF_AMATCH_READER(name, converter)                          \
VALUE                                                               \
rb_amatch_ ## name(self)                                            \
    VALUE self;                                                     \
{                                                                   \
    GET_AMATCH;                                                     \
    return converter(amatch->name);                                 \
}

#define DEF_AMATCH_WRITER(name, type, converter)                    \
VALUE                                                               \
rb_amatch_ ## name ## _set(self, value)                             \
    VALUE self;                                                     \
    VALUE value;                                                    \
{                                                                   \
    GET_AMATCH;                                                     \
    Check_Type(value, type);                                        \
    amatch->name = converter(value);                                \
    return Qnil;                                                    \
}

#define DEF_LEVENSHTEIN(name, mode)                                          \
static VALUE                                                                 \
rb_amatch_ ## name(argc, argv, self)                                         \
    int argc;                                                                \
    VALUE *argv;                                                             \
    VALUE self;                                                              \
{                                                                            \
    int i;                                                                   \
    GET_AMATCH;                                                              \
    VALUE result;                                                            \
    result = rb_ary_new2(argc);                                              \
    if (argc == 0)                                                           \
        rb_raise(rb_eArgError, "wrong number of arguments (%d for 1)", argc);\
    if (argc == 1)                                                           \
        return amatch_compute_levenshtein_distance(amatch, argv[0], mode);   \
    for (i = 0; i < argc; i++) {                                             \
        if (TYPE(argv[i]) != T_STRING) {                                     \
            rb_raise(rb_eTypeError,                                          \
                "argument #%d has to be a string (%s given)", i + 1,         \
                NIL_P(argv[i]) ?                                             \
                    "NilClass" : rb_class2name(CLASS_OF(argv[i])));          \
        }                                                                    \
        rb_ary_push(result,                                                  \
            amatch_compute_levenshtein_distance(amatch, argv[i], mode));     \
    }                                                                        \
    return result;                                                           \
}

static VALUE rb_cAmatch;

typedef struct AmatchStruct {
    int     subw;
    int     delw;
    int     insw;
    char    *pattern;
    char    pattern_length;
    char    **pairs;
} Amatch;

static Amatch *Amatch_allocate()
{
    Amatch *amatch = ALLOC(Amatch);
    MEMZERO(amatch, Amatch, 1);
    return amatch;
}

void
amatch_resetw(amatch)
    Amatch *amatch;
{
    amatch->subw    = 1;
    amatch->delw    = 1;
    amatch->insw    = 1;
}

/*
 * Levenshtein edit distances are computed here:
 */

enum { MATCH = 1, MATCHR, SEARCH, SEARCHR, COMPARE, COMPARER };

static VALUE
amatch_compute_levenshtein_distance(amatch, string, mode)
    Amatch *amatch;
    VALUE string;
    char mode;
{
    static VALUE result;
    int string_len;
    char *string_ptr;
    Vector *v[2];
    int weight,i, j, tmpi;
    int c = 0, p = 1;

    Check_Type(string, T_STRING);
    string_ptr = RSTRING(string)->ptr;
    string_len = RSTRING(string)->len;

    v[0] = Vector_new(string_len);
    switch (mode) {
        case MATCH:
        case MATCHR:
        case COMPARE:
        case COMPARER:
            for (i = 0; i <= v[0]->len; i++) v[0]->ptr[i] = i * amatch->insw;
            break;
        case SEARCH:
        case SEARCHR:
            for (i = 0; i <= v[0]->len; i++) v[0]->ptr[i] = 0;
            break;
        default:
            rb_raise(rb_eFatal,
                "unknown mode in amatch_compute_levenshtein_distance");
    }

    v[1] = Vector_new(string_len);
    for (i = 1; i <= amatch->pattern_length; i++) {
        c = i % 2;                /* current row */
        p = (i - 1) % 2;          /* previous row */
        v[c]->ptr[0] = i * amatch->delw;    /* first column */
        for (j = 1; j <= string_len; j++) {
            /* Bellman's principle of optimality: */
            weight = v[p]->ptr[j - 1] +
                (amatch->pattern[i - 1] == string_ptr[j - 1] ? 0 : amatch->subw);
             if (weight > v[p]->ptr[j] + 1) {
                 weight = v[p]->ptr[j] + amatch->delw;
             }
            if (weight > v[c]->ptr[j - 1] + 1) {
                weight = v[c]->ptr[j - 1] + amatch->insw;
            }
            v[c]->ptr[j] = weight;
        }
    }
    switch (mode) {
        case MATCH:
            result = INT2FIX(vector_last(v[c]));
            break;
        case MATCHR:
            result = rb_float_new(
                (double) vector_last(v[c]) / amatch->pattern_length
            );
            break;
        case SEARCH:
            tmpi = vector_minimum(v[c]);
            result = tmpi < 0 ? INT2FIX(amatch->pattern_length) : INT2FIX(tmpi);
            break;
        case SEARCHR:
            tmpi = vector_minimum(v[c]);
            result = rb_float_new(
                tmpi < 0 ? 1.0 : (double) tmpi / amatch->pattern_length
            );
            break;
        case COMPARE:
            result = INT2FIX((string_len < amatch->pattern_length ? -1 : 1) *
                vector_last(v[c]));
            break;
        case COMPARER:
            result = rb_float_new((double)
                (string_len < amatch->pattern_length ? -1 : 1)     *
                vector_last(v[c]) / amatch->pattern_length);
            break;
        default:
            rb_raise(rb_eFatal,
                "unknown mode in amatch_compute_levenshtein_distance");
    }
    vector_destroy(v[0]);
    vector_destroy(v[1]);
    return result;
}

static VALUE
amatch_compute_pair_distance(amatch, string)
    Amatch *amatch;
    VALUE string;
{
    static VALUE result;
    int string_len;
    char *string_ptr;
    Vector *v[2];
    int weight,i, j, tmpi;
    int c = 0, p = 1;

    Check_Type(string, T_STRING);
    string_ptr = RSTRING(string)->ptr;
    string_len = RSTRING(string)->len;

    if (amatch->pairs) {
    }
    return result;
}

/*
 * Ruby API
 */

static void rb_amatch_free(Amatch *amatch)
{
    free(amatch->pattern);
    MEMZERO(amatch, Amatch, 1);
    free(amatch);
}

static VALUE
rb_amatch_s_allocate(klass)
    VALUE klass;
{
    Amatch *amatch = Amatch_allocate();
    return Data_Wrap_Struct(klass, NULL, rb_amatch_free, amatch);
}

void amatch_pattern_set(amatch, pattern)
    Amatch *amatch;
    VALUE pattern;
{
    Check_Type(pattern, T_STRING);
    free(amatch->pattern);
    amatch->pattern = ALLOC_N(char, RSTRING(pattern)->len);
    MEMCPY(amatch->pattern, RSTRING(pattern)->ptr, char, RSTRING(pattern)->len);
    amatch->pattern_length = RSTRING(pattern)->len;
}
  
VALUE
rb_amatch_pattern(self)
    VALUE self;
{
    GET_AMATCH;
    return rb_str_new(amatch->pattern, amatch->pattern_length);
}

VALUE
rb_amatch_pattern_set(self, pattern)
    VALUE self;
    VALUE pattern;
{
    GET_AMATCH;
    amatch_pattern_set(amatch, pattern);
    return Qnil;
}

DEF_AMATCH_READER(subw, INT2FIX)
DEF_AMATCH_READER(delw, INT2FIX)
DEF_AMATCH_READER(insw, INT2FIX)

DEF_AMATCH_WRITER(subw, T_FIXNUM, FIX2INT)
DEF_AMATCH_WRITER(delw, T_FIXNUM, FIX2INT)
DEF_AMATCH_WRITER(insw, T_FIXNUM, FIX2INT)

static VALUE
rb_amatch_resetw(self) VALUE self;
{
    GET_AMATCH;
    amatch_resetw(amatch);
    return Qtrue;
}

static VALUE
rb_amatch_initialize(self, pattern)
    VALUE self;
    VALUE pattern;
{
    GET_AMATCH;
    amatch_pattern_set(amatch, pattern);
    amatch_resetw(amatch);
    return self;
}

DEF_LEVENSHTEIN(match, MATCH)
DEF_LEVENSHTEIN(matchr, MATCHR)
DEF_LEVENSHTEIN(compare, COMPARE)
DEF_LEVENSHTEIN(comparer, COMPARER)
DEF_LEVENSHTEIN(search, SEARCH)
DEF_LEVENSHTEIN(searchr, SEARCHR)

static VALUE
rb_amatch_pair_distance(argc, argv, self)
    int argc;
    VALUE *argv;
    VALUE self;
{                                                                            
    int i;                                                                   
    GET_AMATCH;                                                              
    VALUE result;                                                            
    result = rb_ary_new2(argc);                                              
    if (argc == 0)                                                           
        rb_raise(rb_eArgError, "wrong number of arguments (%d for 1)", argc);
    if (argc == 1)                                                           
        return amatch_compute_pair_distance(amatch, argv[0]);   
    for (i = 0; i < argc; i++) {                                             
        if (TYPE(argv[i]) != T_STRING) {                                     
            rb_raise(rb_eTypeError,                                          
                "argument #%d has to be a string (%s given)", i + 1,         
                NIL_P(argv[i]) ?                                             
                    "NilClass" : rb_class2name(CLASS_OF(argv[i])));          
        }                                                                    
        rb_ary_push(result, amatch_compute_pair_distance(amatch, argv[i]));     
    }                                                                        
    return result;                                                           
}

void
Init_amatch()
{
    rb_cAmatch = rb_define_class("Amatch", rb_cObject);
    rb_define_alloc_func(rb_cAmatch, rb_amatch_s_allocate);
    rb_define_method(rb_cAmatch, "initialize", rb_amatch_initialize, 1);

    AMATCH_ACCESSOR(subw);
    AMATCH_ACCESSOR(delw);
    AMATCH_ACCESSOR(insw);
    AMATCH_ACCESSOR(pattern);
    rb_define_method(rb_cAmatch, "resetw", rb_amatch_resetw, 0);

    rb_define_method(rb_cAmatch, "match", rb_amatch_match, -1);
    rb_define_method(rb_cAmatch, "matchr", rb_amatch_matchr, -1);
    rb_define_method(rb_cAmatch, "compare", rb_amatch_compare, -1);
    rb_define_method(rb_cAmatch, "comparer", rb_amatch_comparer, -1);
    rb_define_method(rb_cAmatch, "search", rb_amatch_search, -1);
    rb_define_method(rb_cAmatch, "searchr", rb_amatch_searchr, -1);
    rb_define_method(rb_cAmatch, "pair_distance", rb_amatch_pair_distance, -1);
}
    /* vim: set et cin sw=4 ts=4: */
