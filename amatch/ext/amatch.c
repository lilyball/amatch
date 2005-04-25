#include "ruby.h"
#include "pair.h"

static VALUE rb_cAmatch;
static ID id_split, id_to_f;

/* Macromania goes here: */

#define GET_AMATCH                          \
    Amatch *amatch;                         \
    Data_Get_Struct(self, Amatch, amatch)

#define AMATCH_ACCESSOR(name)                                               \
    rb_define_method(rb_cAmatch, #name, rb_amatch_ ## name, 0);             \
    rb_define_method(rb_cAmatch, #name"=", rb_amatch_ ## name ## _set, 1)

#define DEF_AMATCH_READER(name, converter)                          \
VALUE                                                               \
rb_amatch_ ## name(self)                                            \
    VALUE self;                                                     \
{                                                                   \
    GET_AMATCH;                                                     \
    return converter(amatch->name);                                 \
}

#define DEF_AMATCH_WRITER(name, type, caster, converter, check)     \
VALUE                                                               \
rb_amatch_ ## name ## _set(self, value)                             \
    VALUE self;                                                     \
    VALUE value;                                                    \
{                                                                   \
    type value_ ## type;                                            \
    GET_AMATCH;                                                     \
    caster(value);                                                  \
    value_ ## type = converter(value);                              \
    if (!(value_ ## type check))                                    \
        rb_raise(rb_eTypeError, "check of value " #check " failed");\
    amatch->name = value_ ## type;                                  \
    return Qnil;                                                    \
}

#define CAST2FLOAT(obj) \
    if (TYPE(obj) != T_FLOAT && rb_respond_to(obj, id_to_f))    \
            obj = rb_funcall(obj, id_to_f, 0, 0);               \
        else                                                    \
            Check_Type(obj, T_FLOAT)
#define FLOAT2C(obj) RFLOAT(obj)->value

#define OPTIMIZE_TIME                                   \
    if (amatch->pattern_len < RSTRING(string)->len) {   \
        a_ptr = amatch->pattern;                        \
        a_len = amatch->pattern_len;                    \
        b_ptr = RSTRING(string)->ptr;                   \
        b_len = RSTRING(string)->len;                   \
    } else {                                            \
        a_ptr = RSTRING(string)->ptr;                   \
        a_len = RSTRING(string)->len;                   \
        b_ptr = amatch->pattern;                        \
        b_len = amatch->pattern_len;                    \
    }

#define DONT_OPTIMIZE                                   \
        a_ptr = amatch->pattern;                        \
        a_len = amatch->pattern_len;                    \
        b_ptr = RSTRING(string)->ptr;                   \
        b_len = RSTRING(string)->len;                   \

/*
 * The core object of the Amatch class
 */

typedef struct AmatchStruct {
    double      substitution;
    double      deletion;
    double      insertion;
    char        *pattern;
    char        pattern_len;
    PairArray   *pattern_pair_array;
} Amatch;

static Amatch *Amatch_allocate()
{
    Amatch *amatch = ALLOC(Amatch);
    MEMZERO(amatch, Amatch, 1);
    return amatch;
}

static void amatch_reset_weights(amatch)
    Amatch *amatch;
{
    amatch->substitution = 1;
    amatch->deletion     = 1;
    amatch->insertion    = 1;
}

/*
 * Levenshtein edit distances are computed here:
 */

#define COMPUTE_LEVENSHTEIN_DISTANCES                                       \
    for (i = 1, c = 0, p = 1; i <= a_len; i++) {                            \
        c = i % 2;                      /* current row */                   \
        p = (i + 1) % 2;                /* previous row */                  \
        v[c][0] = i * amatch->deletion; /* first column */                  \
        for (j = 1; j <= b_len; j++) {                                      \
            /* Bellman's principle of optimality: */                        \
            weight = v[p][j - 1] +                                          \
                (a_ptr[i - 1] == b_ptr[j - 1] ? 0 : amatch->substitution);  \
            if (weight > v[p][j] + amatch->insertion) {                     \
                 weight = v[p][j] + amatch->insertion;                      \
            }                                                               \
            if (weight > v[c][j - 1] + amatch->deletion) {                  \
                weight = v[c][j - 1] + amatch->deletion;                    \
            }                                                               \
            v[c][j] = weight;                                               \
        }                                                                   \
        p = c;                                                              \
        c = (c + 1) % 2;                                                    \
    }

static VALUE amatch_levenshtein_match(Amatch *amatch, VALUE string)
{
    VALUE result;
    char *a_ptr, *b_ptr;
    int a_len, b_len;
    double *v[2] = { NULL, NULL }, weight;
    int  i, j, c, p;

    Check_Type(string, T_STRING);
    DONT_OPTIMIZE

    v[0] = ALLOC_N(double, b_len + 1);
    for (i = 0; i <= b_len; i++)
        v[0][i] = i * amatch->deletion;
    v[1] = ALLOC_N(double, b_len + 1);
    for (i = 0; i <= b_len; i++)
        v[1][i] = i * amatch->deletion;

    COMPUTE_LEVENSHTEIN_DISTANCES

    result = rb_float_new(v[p][b_len]);
    free(v[0]);
    free(v[1]);
    return result;
}

static VALUE amatch_levenshtein_search(Amatch *amatch, VALUE string)
{
    VALUE result;
    char *a_ptr, *b_ptr;
    int a_len, b_len;
    double *v[2] = { NULL, NULL }, weight, min;
    int  i, j, c, p;

    Check_Type(string, T_STRING);
    DONT_OPTIMIZE

    v[0] = ALLOC_N(double, b_len + 1);
    MEMZERO(v[0], double, b_len + 1);
    v[1] = ALLOC_N(double, b_len + 1);
    MEMZERO(v[1], double, b_len + 1);

    COMPUTE_LEVENSHTEIN_DISTANCES

    for (i = 0, min = a_len; i <= b_len; i++) {
        if (v[p][i] < min) min = v[p][i];
    }
    result = rb_float_new(min);
    free(v[0]);
    free(v[1]);
    
    return result;
}

/*
 * Pair distances are computed here:
 */

static VALUE amatch_pair_distance(
        Amatch *amatch, VALUE string, VALUE regexp, int use_regexp)
{
    double result;
    VALUE tokens;
    PairArray *pair_array;
    
    Check_Type(string, T_STRING);
    if (!NIL_P(regexp) || use_regexp) {
        tokens = rb_funcall(
            rb_str_new(amatch->pattern, amatch->pattern_len),
            id_split, 1, regexp
        );
        if (!amatch->pattern_pair_array) {
            amatch->pattern_pair_array = PairArray_new(tokens);
        } else {
            pair_array_reactivate(amatch->pattern_pair_array);
        }
        tokens = rb_funcall(string, id_split, 1, regexp);
        pair_array = PairArray_new(tokens);
    } else {
        VALUE tmp = rb_str_new(amatch->pattern, amatch->pattern_len);
        tokens = rb_ary_new4(1, &tmp);
        if (!amatch->pattern_pair_array) {
            amatch->pattern_pair_array = PairArray_new(tokens);
        } else {
            pair_array_reactivate(amatch->pattern_pair_array);
        }
        tokens = rb_ary_new4(1, &string);
        pair_array = PairArray_new(tokens);
    }
    result = pair_array_match(amatch->pattern_pair_array, pair_array);
    pair_array_destroy(pair_array);
    return rb_float_new(result);
}

/*
 * Hamming distances are computed here:
 */

static VALUE amatch_hamming(Amatch *amatch, VALUE string)
{
    char *a_ptr, *b_ptr;
    int a_len, b_len;
    int i, result;
    
    Check_Type(string, T_STRING);
    OPTIMIZE_TIME
    for (i = 0, result = b_len - a_len; i < a_len; i++) {
        if (i >= b_len) {
            result +=  a_len - b_len;
            break;
        }
        if (b_ptr[i] != a_ptr[i]) result++;
    }
    return INT2FIX(result);
}

/*
 * Longest Common Subsequence computation
 */

static VALUE amatch_longest_subsequence(Amatch *amatch, VALUE string)
{
    char *a_ptr, *b_ptr;
    int a_len, b_len;
    int result, c, p, i, j, *l[2];
    
    Check_Type(string, T_STRING);
    OPTIMIZE_TIME

    if (a_len == 0 || b_len == 0) return INT2FIX(0);

    l[0] = ALLOC_N(int, b_len + 1); 
    l[1] = ALLOC_N(int, b_len + 1);
    for (i = a_len, c = 0, p = 1; i >= 0; i--) {
        for (j = b_len; j >= 0; j--) {
            if (i == a_len || j == b_len) {
                l[c][j] = 0;
            } else if (a_ptr[i] == b_ptr[j]) {
                l[c][j] = 1 + l[p][j + 1];
            } else {
                int x = l[p][j], y = l[c][j + 1];
                if (x > y) l[c][j] = x; else l[c][j] = y;
            }
        }
        p = c;
        c = (c + 1) % 2;
    }
    result = l[p][0];
    free(l[0]);
    free(l[1]);
    return INT2FIX(result);
}

/*
 * Longest Common Subsequence computation
 */

static VALUE amatch_longest_substring(Amatch *amatch, VALUE string)
{
    char *a_ptr, *b_ptr;
    int a_len, b_len;
    int result, c, p, i, j, *l[2];
    
    Check_Type(string, T_STRING);
    OPTIMIZE_TIME

    if (a_len == 0 || b_len == 0) return INT2FIX(0);

    l[0] = ALLOC_N(int, b_len); 
    MEMZERO(l[0], int, b_len);
    l[1] = ALLOC_N(int, b_len);
    MEMZERO(l[1], int, b_len);
    result = 0;
    for (i = 0, c = 0, p = 1; i < a_len; i++) {
        for (j = 0; j < b_len; j++) {
            if (a_ptr[i] == b_ptr[j]) {
                l[c][j] = j == 0 ? 1 : 1 + l[p][j - 1];
                if (l[c][j] > result) result = l[c][j];
            } else {
                l[c][j] = 0;
            }
        }
        p = c;
        c = (c + 1) % 2;
    }
    free(l[0]);
    free(l[1]);
    return INT2FIX(result);
}

/*
 * A few helper functions go here:
 */

static VALUE iterate_strings(VALUE self, VALUE strings,
    VALUE (*match_function) (Amatch *amatch, VALUE strings))
{
    GET_AMATCH;
    if (TYPE(strings) == T_STRING) {
        return match_function(amatch, strings);
    } else {
        Check_Type(strings, T_ARRAY);
        int i;
        VALUE result = rb_ary_new2(RARRAY(strings)->len);
        for (i = 0; i < RARRAY(strings)->len; i++) {
            VALUE string = rb_ary_entry(strings, i);
            if (TYPE(string) != T_STRING) {
                rb_raise(rb_eTypeError,
                    "array has to contain only strings (%s given)",
                    NIL_P(string) ?
                        "NilClass" : rb_class2name(CLASS_OF(string)));
            }
            rb_ary_push(result, match_function(amatch, string));
        }
        return result;
    }
}

/*
 * Ruby API
 */

static void rb_amatch_free(Amatch *amatch)
{
    MEMZERO(amatch->pattern, char, amatch->pattern_len);
    free(amatch->pattern);
    MEMZERO(amatch, Amatch, 1);
    free(amatch);
}

static VALUE rb_amatch_s_allocate(VALUE klass)
{
    Amatch *amatch = Amatch_allocate();
    return Data_Wrap_Struct(klass, NULL, rb_amatch_free, amatch);
}

static void amatch_pattern_set(Amatch *amatch, VALUE pattern)
{
    Check_Type(pattern, T_STRING);
    free(amatch->pattern);
    amatch->pattern_len = RSTRING(pattern)->len;
    amatch->pattern = ALLOC_N(char, amatch->pattern_len);
    MEMCPY(amatch->pattern, RSTRING(pattern)->ptr, char, RSTRING(pattern)->len);
}
  
static VALUE rb_amatch_pattern(VALUE self)
{
    GET_AMATCH;
    return rb_str_new(amatch->pattern, amatch->pattern_len);
}

static VALUE rb_amatch_pattern_set(VALUE self, VALUE pattern)
{
    GET_AMATCH;
    amatch_pattern_set(amatch, pattern);
    return Qnil;
}

DEF_AMATCH_READER(substitution, rb_float_new)
DEF_AMATCH_READER(deletion, rb_float_new)
DEF_AMATCH_READER(insertion, rb_float_new)

DEF_AMATCH_WRITER(substitution, float, CAST2FLOAT, FLOAT2C, >= 0)
DEF_AMATCH_WRITER(deletion, float, CAST2FLOAT, FLOAT2C, >= 0)
DEF_AMATCH_WRITER(insertion, float, CAST2FLOAT, FLOAT2C, >= 0)

static VALUE rb_amatch_reset_weights(VALUE self)
{
    GET_AMATCH;
    amatch_reset_weights(amatch);
    return self;
}

static VALUE rb_amatch_initialize(VALUE self, VALUE pattern)
{
    GET_AMATCH;
    amatch_pattern_set(amatch, pattern);
    amatch_reset_weights(amatch);
    return self;
}

VALUE rb_amatch_new(VALUE klass, VALUE pattern)
{
    VALUE obj = rb_amatch_s_allocate(klass);
    rb_amatch_initialize(obj, pattern);
    return obj;
}

static VALUE rb_amatch_levenshtein_match(VALUE self, VALUE strings)
{                                                                            
    return iterate_strings(self, strings, amatch_levenshtein_match);
}

static VALUE rb_str_levenshtein_match(VALUE self, VALUE strings)
{
    VALUE amatch = rb_amatch_new(rb_cAmatch, self);
    return rb_amatch_levenshtein_match(amatch, strings);
}

static VALUE rb_amatch_levenshtein_search(VALUE self, VALUE strings)
{                                                                            
    return iterate_strings(self, strings, amatch_levenshtein_search);
}

static VALUE rb_str_levenshtein_search(VALUE self, VALUE strings)
{
    VALUE amatch = rb_amatch_new(rb_cAmatch, self);
    return rb_amatch_levenshtein_search(amatch, strings);
}

static VALUE rb_amatch_pair_distance(int argc, VALUE *argv, VALUE self)
{                                                                            
    VALUE result, strings, regexp = Qnil;
    int use_regexp;
    GET_AMATCH;

    rb_scan_args(argc, argv, "11", &strings, &regexp);
    use_regexp = NIL_P(regexp) && argc != 2;
    if (TYPE(strings) == T_STRING) {
        result = amatch_pair_distance(amatch, strings, regexp, use_regexp);
    } else {
        Check_Type(strings, T_ARRAY);
        int i;
        result = rb_ary_new2(RARRAY(strings)->len);
        for (i = 0; i < RARRAY(strings)->len; i++) {
            VALUE string = rb_ary_entry(strings, i);
            if (TYPE(string) != T_STRING) {
                rb_raise(rb_eTypeError,
                    "array has to contain only strings (%s given)",
                    NIL_P(string) ?
                        "NilClass" : rb_class2name(CLASS_OF(string)));
            }
            rb_ary_push(result,
                amatch_pair_distance(amatch, string, regexp, use_regexp));
        }
    }
    pair_array_destroy(amatch->pattern_pair_array);
    amatch->pattern_pair_array = NULL;
    return result;
}

static VALUE rb_str_pair_distance(VALUE self, VALUE strings)
{
    VALUE amatch = rb_amatch_new(rb_cAmatch, self);
    return rb_amatch_pair_distance(1, &strings, amatch);
}

static VALUE rb_amatch_hamming(VALUE self, VALUE strings)
{                                                                            
    return iterate_strings(self, strings, amatch_hamming);
}

static VALUE rb_str_hamming(VALUE self, VALUE strings)
{
    VALUE amatch = rb_amatch_new(rb_cAmatch, self);
    return rb_amatch_hamming(amatch, strings);
}

static VALUE rb_amatch_longest_subsequence(VALUE self, VALUE strings)
{                                                                            
    return iterate_strings(self, strings, amatch_longest_subsequence);
}

static VALUE rb_str_longest_subsequence(VALUE self, VALUE strings)
{                                                                            
    VALUE amatch = rb_amatch_new(rb_cAmatch, self);
    return rb_amatch_longest_subsequence(amatch, strings);
}

static VALUE rb_amatch_longest_substring(VALUE self, VALUE strings)
{                                                                            
    return iterate_strings(self, strings, amatch_longest_substring);
}

static VALUE rb_str_longest_substring(VALUE self, VALUE strings)
{                                                                            
    VALUE amatch = rb_amatch_new(rb_cAmatch, self);
    return rb_amatch_longest_substring(amatch, strings);
}

void Init_amatch()
{
    rb_cAmatch = rb_define_class("Amatch", rb_cObject);
    rb_define_alloc_func(rb_cAmatch, rb_amatch_s_allocate);
    rb_define_method(rb_cAmatch, "initialize", rb_amatch_initialize, 1);

    AMATCH_ACCESSOR(pattern);
    AMATCH_ACCESSOR(substitution);
    AMATCH_ACCESSOR(deletion);
    AMATCH_ACCESSOR(insertion);
    rb_define_method(rb_cAmatch, "reset_weights", rb_amatch_reset_weights, 0);

    rb_define_method(rb_cAmatch, "levenshtein_match", rb_amatch_levenshtein_match, 1);
    rb_define_method(rb_cString, "levenshtein_match", rb_str_levenshtein_match, 1);
    rb_define_method(rb_cAmatch, "levenshtein_search", rb_amatch_levenshtein_search, 1);
    rb_define_method(rb_cString, "levenshtein_search", rb_str_levenshtein_search, 1);
    rb_define_method(rb_cAmatch, "hamming", rb_amatch_hamming, 1);
    rb_define_method(rb_cString, "hamming", rb_str_hamming, 1);
    rb_define_method(rb_cAmatch, "pair_distance", rb_amatch_pair_distance, -1);
    rb_define_method(rb_cString, "pair_distance", rb_str_pair_distance, 1);
    rb_define_method(rb_cAmatch, "longest_subsequence", rb_amatch_longest_subsequence, 1);
    rb_define_method(rb_cString, "longest_subsequence", rb_str_longest_subsequence, 1);
    rb_define_method(rb_cAmatch, "longest_substring", rb_amatch_longest_substring, 1);
    rb_define_method(rb_cString, "longest_substring", rb_str_longest_substring, 1);
    id_split = rb_intern("split");
    id_to_f = rb_intern("to_f");
}
    /* vim: set et cin sw=4 ts=4: */
