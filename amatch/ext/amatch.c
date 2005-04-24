#include "ruby.h"
#include "vector.h"
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

#define DEF_LEVENSHTEIN(name, mode)                             \
static VALUE                                                    \
rb_amatch_ ## name(VALUE self, VALUE strings)                   \
{                                                               \
    return levenshtein_iterate_strings(self, strings, mode);    \
}

/*
 * The core object of the Amatch class
 */

typedef struct AmatchStruct {
    double      subw;
    double      delw;
    double      insw;
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

static void amatch_resetw(amatch)
    Amatch *amatch;
{
    amatch->subw    = 1;
    amatch->delw    = 1;
    amatch->insw    = 1;
}

/*
 * Levenshtein edit distances are computed here:
 */

enum { L_MATCH = 1, L_MATCHR, SEARCH, SEARCHR, COMPARE, COMPARER };

static VALUE amatch_compute_levenshtein_distance(
        Amatch *amatch, VALUE string, char mode)
{
    VALUE result;
    int string_len;
    char *string_ptr;
    Vector *v[2];
    double weight, tmp;
    int  i, j;
    int c = 0, p = 1;

    Check_Type(string, T_STRING);
    string_ptr = RSTRING(string)->ptr;
    string_len = RSTRING(string)->len;

    v[0] = Vector_new(string_len);
    switch (mode) {
        case L_MATCH:
        case L_MATCHR:
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
    for (i = 1; i <= amatch->pattern_len; i++) {
        c = i % 2;                /* current row */
        p = (i + 1) % 2;          /* previous row */
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
        case L_MATCH:
            result = rb_float_new(vector_last(v[c]));
            break;
        case L_MATCHR:
            result = rb_float_new(
                (double) vector_last(v[c]) / amatch->pattern_len
            );
            break;
        case SEARCH:
            tmp = vector_minimum(v[c]);
            result = tmp < 0 ?
                rb_float_new((double) amatch->pattern_len) : rb_float_new(tmp);
            break;
        case SEARCHR:
            tmp = vector_minimum(v[c]);
            result = rb_float_new(
                tmp < 0 ? 1.0 : (double) tmp / amatch->pattern_len
            );
            break;
        case COMPARE:
            result = rb_float_new(
                (double) (string_len < amatch->pattern_len ? -1 : 1) *
                vector_last(v[c]));
            break;
        case COMPARER:
            result = rb_float_new((double)
                (string_len < amatch->pattern_len ? -1 : 1)     *
                vector_last(v[c]) / amatch->pattern_len);
            break;
        default:
            rb_raise(rb_eFatal,
                "unknown mode in amatch_compute_levenshtein_distance");
    }
    vector_destroy(v[0]);
    vector_destroy(v[1]);
    return result;
}
/*
 * Pair distances are computed here:
 */

static VALUE amatch_compute_pair_distance(
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
    char *string_ptr;
    int i, string_len;
    int result = 0;
    
    Check_Type(string, T_STRING);
    string_ptr = RSTRING(string)->ptr;
    string_len = RSTRING(string)->len;
    if (string_len > amatch->pattern_len) {
        result += string_len - amatch->pattern_len;
    }
    for (i = 0; i < amatch->pattern_len; i++) {
        if (i >= string_len) {
            result +=  amatch->pattern_len - string_len;
            break;
        }
        if (string_ptr[i] != amatch->pattern[i]) result++;
    }
    return INT2FIX(result);
}

static VALUE amatch_hammingr(Amatch *amatch, VALUE string)
{
    char *string_ptr;
    int i, string_len;
    int result = 0;
    
    Check_Type(string, T_STRING);
    string_ptr = RSTRING(string)->ptr;
    string_len = RSTRING(string)->len;
    if (string_len > amatch->pattern_len) {
        result += string_len - amatch->pattern_len;
    }
    for (i = 0; i < amatch->pattern_len; i++) {
        if (i >= string_len) {
            result +=  amatch->pattern_len - string_len;
            break;
        }
        if (string_ptr[i] != amatch->pattern[i]) result++;
    }
    return rb_float_new((double) result / amatch->pattern_len);
}

static VALUE amatch_lcs_subsequence(Amatch *amatch, VALUE string)
{
    char *a_ptr, *b_ptr;
    int result, c, p, i, j, a_len, b_len, *l[2];
    
    Check_Type(string, T_STRING);
    if (amatch->pattern_len < RSTRING(string)->len) {
        a_ptr = amatch->pattern;
        a_len = amatch->pattern_len;
        b_ptr = RSTRING(string)->ptr;
        b_len = RSTRING(string)->len;
    } else {
        a_ptr = RSTRING(string)->ptr;
        a_len = RSTRING(string)->len;
        b_ptr = amatch->pattern;
        b_len = amatch->pattern_len;
    }

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

static VALUE levenshtein_iterate_strings(VALUE self, VALUE strings, char mode)
{
    GET_AMATCH;
    if (TYPE(strings) == T_STRING) {
        return amatch_compute_levenshtein_distance(amatch, strings, mode);
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
            rb_ary_push(result,
                amatch_compute_levenshtein_distance(amatch, string, mode));
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

DEF_AMATCH_READER(subw, rb_float_new)
DEF_AMATCH_READER(delw, rb_float_new)
DEF_AMATCH_READER(insw, rb_float_new)

DEF_AMATCH_WRITER(subw, float, CAST2FLOAT, FLOAT2C, >= 0)
DEF_AMATCH_WRITER(delw, float, CAST2FLOAT, FLOAT2C, >= 0)
DEF_AMATCH_WRITER(insw, float, CAST2FLOAT, FLOAT2C, >= 0)

static VALUE rb_amatch_resetw(VALUE self)
{
    GET_AMATCH;
    amatch_resetw(amatch);
    return Qtrue;
}

static VALUE rb_amatch_initialize(VALUE self, VALUE pattern)
{
    GET_AMATCH;
    amatch_pattern_set(amatch, pattern);
    amatch_resetw(amatch);
    return self;
}

DEF_LEVENSHTEIN(match, L_MATCH)
DEF_LEVENSHTEIN(matchr, L_MATCHR)
DEF_LEVENSHTEIN(compare, COMPARE)
DEF_LEVENSHTEIN(comparer, COMPARER)
DEF_LEVENSHTEIN(search, SEARCH)
DEF_LEVENSHTEIN(searchr, SEARCHR)

static VALUE rb_amatch_pair_distance(int argc, VALUE *argv, VALUE self)
{                                                                            
    VALUE result, strings, regexp = Qnil;
    int use_regexp;
    GET_AMATCH;

    rb_scan_args(argc, argv, "11", &strings, &regexp);
    use_regexp = NIL_P(regexp) && argc != 2;
    if (TYPE(strings) == T_STRING) {
        result = amatch_compute_pair_distance(amatch, strings, regexp, use_regexp);
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
                amatch_compute_pair_distance(amatch, string, regexp, use_regexp));
        }
    }
    pair_array_destroy(amatch->pattern_pair_array);
    amatch->pattern_pair_array = NULL;
    return result;
}

static VALUE rb_amatch_hamming(VALUE self, VALUE strings)
{                                                                            
    return iterate_strings(self, strings, amatch_hamming);
}

static VALUE rb_amatch_hammingr(VALUE self, VALUE strings)
{                                                                            
    return iterate_strings(self, strings, amatch_hammingr);
}

static VALUE rb_amatch_lc_subsequence(VALUE self, VALUE strings)
{                                                                            
    return iterate_strings(self, strings, amatch_lcs_subsequence);
}


void Init_amatch()
{
    rb_cAmatch = rb_define_class("Amatch", rb_cObject);
    rb_define_alloc_func(rb_cAmatch, rb_amatch_s_allocate);
    rb_define_method(rb_cAmatch, "initialize", rb_amatch_initialize, 1);

    AMATCH_ACCESSOR(subw);
    AMATCH_ACCESSOR(delw);
    AMATCH_ACCESSOR(insw);
    AMATCH_ACCESSOR(pattern);
    rb_define_method(rb_cAmatch, "resetw", rb_amatch_resetw, 0);
    rb_define_method(rb_cAmatch, "reset_weights", rb_amatch_resetw, 0);

    rb_define_method(rb_cAmatch, "match", rb_amatch_match, 1);
    rb_define_method(rb_cAmatch, "matchr", rb_amatch_matchr, 1);
    rb_define_method(rb_cAmatch, "compare", rb_amatch_compare, 1);
    rb_define_method(rb_cAmatch, "comparer", rb_amatch_comparer, 1);
    rb_define_method(rb_cAmatch, "search", rb_amatch_search, 1);
    rb_define_method(rb_cAmatch, "searchr", rb_amatch_searchr, 1);
    rb_define_method(rb_cAmatch, "hamming", rb_amatch_hamming, 1);
    rb_define_method(rb_cAmatch, "hammingr", rb_amatch_hammingr, 1);
    rb_define_method(rb_cAmatch, "pair_distance", rb_amatch_pair_distance, -1);
    rb_define_method(rb_cAmatch, "lc_subsequence", rb_amatch_lc_subsequence, 1);
    id_split = rb_intern("split");
    id_to_f = rb_intern("to_f");
}
    /* vim: set et cin sw=4 ts=4: */
