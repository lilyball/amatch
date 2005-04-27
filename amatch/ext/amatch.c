#include "ruby.h"
#include "pair.h"

/*
 * Document-module: Amatch
 * 
 * TODO
 * 
 */

/*
 * Document-method: pattern
 *
 * call-seq: pattern -> pattern string
 *
 * Returns the current pattern string of this instance.
 */

/*
 * Document-method: pattern=
 *
 * call-seq: pattern=(pattern)
 * 
 * Sets the current pattern string of this instance to <code>pattern</code>.
 */


static VALUE rb_mAmatch, rb_cLevenshtein, rb_cHamming, rb_cPairDistance,
             rb_cLongestSubsequence, rb_cLongestSubstring;

static ID id_split, id_to_f;

#define GET_STRUCT(klass)                 \
    klass *amatch;                        \
    Data_Get_Struct(self, klass, amatch);

#define DEF_ALLOCATOR(type)                                             \
static type *type##_allocate()                                          \
{                                                                       \
    type *obj = ALLOC(type);                                            \
    MEMZERO(obj, type, 1);                                              \
    return obj;                                                         \
}

#define DEF_CONSTRUCTOR(klass, type)                                    \
static VALUE rb_##klass##_s_allocate(VALUE klass2)                      \
{                                                                       \
    type *amatch = type##_allocate();                                   \
    return Data_Wrap_Struct(klass2, NULL, rb_##klass##_free, amatch);   \
}                                                                       \
VALUE rb_##klass##_new(VALUE klass2, VALUE pattern)                     \
{                                                                       \
    VALUE obj = rb_##klass##_s_allocate(klass2);                        \
    rb_##klass##_initialize(obj, pattern);                              \
    return obj;                                                         \
}

#define DEF_RB_FREE(klass, type)                            \
static void rb_##klass##_free(type *amatch)                 \
{                                                           \
    MEMZERO(amatch->pattern, char, amatch->pattern_len);    \
    free(amatch->pattern);                                  \
    MEMZERO(amatch, type, 1);                               \
    free(amatch);                                           \
}

#define DEF_PATTERN_ACCESSOR(type)                              \
static void type##_pattern_set(type *amatch, VALUE pattern)     \
{                                                               \
    Check_Type(pattern, T_STRING);                              \
    free(amatch->pattern);                                      \
    amatch->pattern_len = RSTRING(pattern)->len;                \
    amatch->pattern = ALLOC_N(char, amatch->pattern_len);       \
    MEMCPY(amatch->pattern, RSTRING(pattern)->ptr, char,        \
        RSTRING(pattern)->len);                                 \
}                                                               \
static VALUE rb_##type##_pattern(VALUE self)                    \
{                                                               \
    GET_STRUCT(type)                                            \
    return rb_str_new(amatch->pattern, amatch->pattern_len);    \
}                                                               \
static VALUE rb_##type##_pattern_set(VALUE self, VALUE pattern) \
{                                                               \
    GET_STRUCT(type)                                            \
    type##_pattern_set(amatch, pattern);                        \
    return Qnil;                                                \
}

#define DEF_ITERATE_STRINGS(type)                                   \
static VALUE type##_iterate_strings(type *amatch, VALUE strings,     \
    VALUE (*match_function) (type *amatch, VALUE strings))        \
{                                                                   \
    if (TYPE(strings) == T_STRING) {                                \
        return match_function(amatch, strings);                     \
    } else {                                                        \
        Check_Type(strings, T_ARRAY);                               \
        int i;                                                      \
        VALUE result = rb_ary_new2(RARRAY(strings)->len);           \
        for (i = 0; i < RARRAY(strings)->len; i++) {                \
            VALUE string = rb_ary_entry(strings, i);                \
            if (TYPE(string) != T_STRING) {                         \
                rb_raise(rb_eTypeError,                             \
                    "array has to contain only strings (%s given)", \
                    NIL_P(string) ?                                 \
                        "NilClass" :                                \
                        rb_class2name(CLASS_OF(string)));           \
            }                                                       \
            rb_ary_push(result, match_function(amatch, string));    \
        }                                                           \
        return result;                                              \
    }                                                               \
}

#define DEF_RB_READER(type, function, name, converter)              \
VALUE function(VALUE self)                                          \
{                                                                   \
    GET_STRUCT(type)                                                \
    return converter(amatch->name);                                 \
}

#define DEF_RB_WRITER(type, function, name, vtype, caster, converter, check)\
VALUE function(VALUE self, VALUE value)                                 \
{                                                                       \
    vtype value_ ## vtype;                                              \
    GET_STRUCT(type)                                                    \
    caster(value);                                                      \
    value_ ## vtype = converter(value);                                 \
    if (!(value_ ## vtype check))                                       \
        rb_raise(rb_eTypeError, "check of value " #check " failed");    \
    amatch->name = value_ ## vtype;                                     \
    return Qnil;                                                        \
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
 * C structures of the Amatch classes
 */

typedef struct GeneralStruct {
    char        *pattern;
    char        pattern_len;
} General;

DEF_ALLOCATOR(General)
DEF_PATTERN_ACCESSOR(General)
DEF_ITERATE_STRINGS(General)

typedef struct LevenshteinStruct {
    char        *pattern;
    char        pattern_len;
    double      substitution;
    double      deletion;
    double      insertion;
} Levenshtein;

DEF_ALLOCATOR(Levenshtein)
DEF_PATTERN_ACCESSOR(Levenshtein)
DEF_ITERATE_STRINGS(Levenshtein)

static void Levenshtein_reset_weights(Levenshtein *self)
{
    self->substitution = 1;
    self->deletion     = 1;
    self->insertion    = 1;
}

typedef struct PairDistanceStruct {
    char        *pattern;
    char        pattern_len;
    PairArray   *pattern_pair_array;
} PairDistance;

DEF_ALLOCATOR(PairDistance)
DEF_PATTERN_ACCESSOR(PairDistance)

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

static VALUE Levenshtein_match(Levenshtein *amatch, VALUE string)
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

static VALUE Levenshtein_search(Levenshtein *amatch, VALUE string)
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

static VALUE PairDistance_match(
        PairDistance *amatch, VALUE string, VALUE regexp, int use_regexp)
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

static VALUE hamming(General *amatch, VALUE string)
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

static VALUE longest_subsequence(General *amatch, VALUE string)
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

static VALUE amatch_LongestSubstring(General *amatch, VALUE string)
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
 * Ruby API
 */

/* 
 * Document-class: Amatch::Levenshtein
 *
 * TODO
 *  
 */

DEF_RB_FREE(Levenshtein, Levenshtein)

/*
 * Document-method: substitution
 *
 * call-seq: substitution -> weight
 *
 * Returns the weight of the substitution operation, that is used to compute
 * the Levenshtein distance.
 */
DEF_RB_READER(Levenshtein, rb_Levenshtein_substitution, substitution,
    rb_float_new)

/*
 * Document-method: deletion
 *
 * call-seq: deletion -> weight
 *
 * Returns the weight of the deletion operation, that is used to compute
 * the Levenshtein distance.
 */
DEF_RB_READER(Levenshtein, rb_Levenshtein_deletion, deletion,
    rb_float_new)

/*
 * Document-method: insertion
 *
 * call-seq: insertion -> weight
 *
 * Returns the weight of the insertion operation, that is used to compute
 * the Levenshtein distance.
 */
DEF_RB_READER(Levenshtein, rb_Levenshtein_insertion, insertion,
    rb_float_new)

/*
 * Document-method: substitution=
 *
 * call-seq: substitution=(weight)
 *
 * Sets the weight of the substitution operation, that is used to compute
 * the Levenshtein distance, to <code>weight</code>. The <code>weight</code>
 * should be a Float value >= 0.0.
 */
DEF_RB_WRITER(Levenshtein, rb_Levenshtein_substitution_set, substitution,
    float, CAST2FLOAT, FLOAT2C, >= 0)

/*
 * Document-method: deletion=
 *
 * call-seq: deletion=(weight)
 *
 * Sets the weight of the deletion operation, that is used to compute
 * the Levenshtein distance, to <code>weight</code>. The <code>weight</code>
 * should be a Float value >= 0.0.
 */
DEF_RB_WRITER(Levenshtein, rb_Levenshtein_deletion_set, deletion,
    float, CAST2FLOAT, FLOAT2C, >= 0)

/*
 * Document-method: insertion=
 *
 * call-seq: insertion=(weight)
 *
 * Sets the weight of the insertion operation, that is used to compute
 * the Levenshtein distance, to <code>weight</code>. The <code>weight</code>
 * should be a Float value >= 0.0.
 */
DEF_RB_WRITER(Levenshtein, rb_Levenshtein_insertion_set, insertion,
    float, CAST2FLOAT, FLOAT2C, >= 0)

/*
 * Resets all weights (substitution, deletion, and insertion) to 1.0.
 */
static VALUE rb_Levenshtein_reset_weights(VALUE self)
{
    GET_STRUCT(Levenshtein)
    Levenshtein_reset_weights(amatch);
    return self;
}

/*
 * call-seq: new(pattern)
 *
 * Creates a new Amatch::Levenshtein instance from <code>pattern</code>,
 * with all weights initially set to 1.0.
 */
static VALUE rb_Levenshtein_initialize(VALUE self, VALUE pattern)
{
    GET_STRUCT(Levenshtein)
    Levenshtein_pattern_set(amatch, pattern);
    Levenshtein_reset_weights(amatch);
    return self;
}

DEF_CONSTRUCTOR(Levenshtein, Levenshtein)

/*
 * Document-method: pattern
 *
 * call-seq: pattern -> pattern string
 *
 * Returns the current pattern string of this Amatch::Levenshtein instance.
 */

/*
 * Document-method: pattern=
 *
 * call-seq: pattern=(pattern)
 * 
 * Sets the current pattern string of this Amatch::Levenshtein instance to
 * <code>pattern</code>.
 */

/*
 * call-seq: match(strings) -> results
 * 
 * Uses this Amatch::Levenshtein instance to match against
 * <code>strings</code>. <code>strings</code> has to be either a String or an
 * Array of Strings. The returned <code>results</code> are either a Float or an
 * Array of Floats respectively.
 */
static VALUE rb_Levenshtein_match(VALUE self, VALUE strings)
{                                                                            
    GET_STRUCT(Levenshtein)
    return Levenshtein_iterate_strings(amatch, strings, Levenshtein_match);
}

/*
 * TODO
 */
static VALUE rb_str_Levenshtein_match(VALUE self, VALUE strings)
{
    VALUE amatch = rb_Levenshtein_new(rb_cLevenshtein, self);
    return rb_Levenshtein_match(amatch, strings);
}

/*
 * TODO
 */
static VALUE rb_Levenshtein_search(VALUE self, VALUE strings)
{                                                                            
    GET_STRUCT(Levenshtein)
    return Levenshtein_iterate_strings(amatch, strings, Levenshtein_search);
}

/*
 * TODO
 */
static VALUE rb_str_Levenshtein_search(VALUE self, VALUE strings)
{
    VALUE amatch = rb_Levenshtein_new(rb_cLevenshtein, self);
    return rb_Levenshtein_search(amatch, strings);
}

/* 
 * Document-class: Amatch::PairDistance
 *
 * TODO
 *  
 */

DEF_RB_FREE(PairDistance, PairDistance)

/*
 * call-seq: new(pattern)
 *
 * Creates a new Amatch::PairDistance instance from <code>pattern</code>.
 */
static VALUE rb_PairDistance_initialize(VALUE self, VALUE pattern)
{
    GET_STRUCT(PairDistance)
    PairDistance_pattern_set(amatch, pattern);
    return self;
}

DEF_CONSTRUCTOR(PairDistance, PairDistance)

/*
 * call-seq: match(strings) -> results
 * 
 * Uses this Amatch::PairDistance instance to match against
 * <code>strings</code>. <code>strings</code> has to be either a String or an
 * Array of Strings. The returned <code>results</code> are either a Float or an
 * Array of Floats respectively.
 */
static VALUE rb_PairDistance_match(int argc, VALUE *argv, VALUE self)
{                                                                            
    VALUE result, strings, regexp = Qnil;
    int use_regexp;
    GET_STRUCT(PairDistance)

    rb_scan_args(argc, argv, "11", &strings, &regexp);
    use_regexp = NIL_P(regexp) && argc != 2;
    if (TYPE(strings) == T_STRING) {
        result = PairDistance_match(amatch, strings, regexp, use_regexp);
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
                        "NilClass" :
                        rb_class2name(CLASS_OF(string)));
            }
            rb_ary_push(result,
                PairDistance_match(amatch, string, regexp, use_regexp));
        }
    }
    pair_array_destroy(amatch->pattern_pair_array);
    amatch->pattern_pair_array = NULL;
    return result;
}

/*
 * TODO
 */
static VALUE rb_str_pair_distance_match(VALUE self, VALUE strings)
{
    VALUE amatch = rb_PairDistance_new(rb_cLevenshtein, self);
    return rb_PairDistance_match(1, &strings, amatch);
}

/* 
 * Document-class: Amatch::Hamming
 *
 *  This class computes the Hamming distance between two strings.
 *
 *  The Hamming distance between two strings is the number of characters,
 *  that are different. Thus a hamming distance of 0 means an exact
 *  match, a hamming distance of 1 means one character is different, and so
 *  on.
 */

DEF_RB_FREE(Hamming, General)

/*
 * call-seq: new(pattern)
 *
 * Creates a new Amatch::Hamming instance from <code>pattern</code>.
 */
static VALUE rb_Hamming_initialize(VALUE self, VALUE pattern)
{
    GET_STRUCT(General)
    General_pattern_set(amatch, pattern);
    return self;
}

DEF_CONSTRUCTOR(Hamming, General)

/*
 * call-seq: match(strings) -> results
 * 
 * Uses this Amatch::Hamming instance to match against
 * <code>strings</code>, that is compute the hamming distance between
 * <code>pattern</code> and <code>strings</code>. <code>strings</code> has to
 * be either a String or an Array of Strings. The returned <code>results</code>
 * are either a Fixnum or an Array of Fixnums respectively.
 */
static VALUE rb_Hamming_match(VALUE self, VALUE strings)
{                                                                            
    GET_STRUCT(General)
    return General_iterate_strings(amatch, strings, hamming);
}

/*
 * TODO
 */
static VALUE rb_str_hamming_match(VALUE self, VALUE strings)
{
    VALUE amatch = rb_Hamming_new(rb_cLevenshtein, self);
    return rb_Hamming_match(amatch, strings);
}

/* 
 * Document-class: Amatch::LongestSubsequence
 *
 *  This class computes the length of the longest subsequence common to two
 *  strings. A subsequence doesn't have to be contiguous. The longer the common
 *  subsequence is, the more similar the two strings will be.
 *
 *  The longest common subsequence between "test" and "test" is of length 4,
 *  because "test" itself is this subsequence. The longest common subsequence
 *  between "test" and "east" is "e", "s", "t" and the length of the
 *  sequence is 3.
 */
DEF_RB_FREE(LongestSubsequence, General)

/*
 * call-seq: new(pattern)
 *
 * Creates a new Amatch::LongestSubsequence instance from <code>pattern</code>.
 */
static VALUE rb_LongestSubsequence_initialize(VALUE self, VALUE pattern)
{
    GET_STRUCT(General)
    General_pattern_set(amatch, pattern);
    return self;
}

DEF_CONSTRUCTOR(LongestSubsequence, General)

/*
 * call-seq: match(strings) -> results
 * 
 * Uses this Amatch::LongestSubsequence instance to match against
 * <code>strings</code>, that is compute the length of the longest common
 * subsequence. <code>strings</code> has to be either a String or an Array of
 * Strings. The returned <code>results</code> are either a Fixnum or an Array
 * of Fixnums respectively.
 */
static VALUE rb_longest_subsequence_match(VALUE self, VALUE strings)
{                                                                            
    GET_STRUCT(General)
    return General_iterate_strings(amatch, strings, longest_subsequence);
}

/*
 * TODO
 */
static VALUE rb_str_longest_subsequence_match(VALUE self, VALUE strings)
{                                                                            
    VALUE amatch = rb_LongestSubsequence_new(rb_cLevenshtein, self);
    return rb_longest_subsequence_match(amatch, strings);
}

/* 
 * Document-class: Amatch::LongestSubstring
 *
 * TODO
 *  
 */

DEF_RB_FREE(LongestSubstring, General)

/*
 * call-seq: new(pattern)
 *
 * Creates a new Amatch::LongestSubstring instance from <code>pattern</code>.
 */
static VALUE rb_LongestSubstring_initialize(VALUE self, VALUE pattern)
{
    GET_STRUCT(General)
    General_pattern_set(amatch, pattern);
    return self;
}

DEF_CONSTRUCTOR(LongestSubstring, General)

/*
 * call-seq: match(strings) -> results
 * 
 * Uses this Amatch::LongestSubstring instance to match against
 * <code>strings</code>. <code>strings</code> has to be either a String or an
 * Array of Strings. The returned <code>results</code> are either a Fixnum or an
 * Array of Fixnums respectively.
 */
static VALUE rb_LongestSubstring_match_match(VALUE self, VALUE strings)
{
    GET_STRUCT(General)
    return General_iterate_strings(amatch, strings, amatch_LongestSubstring);
}

/*
 * TODO
 */
static VALUE rb_str_longest_substring_match(VALUE self, VALUE strings)
{                                                                            
    VALUE amatch = rb_LongestSubsequence_new(rb_cLevenshtein, self);
    return rb_LongestSubstring_match_match(amatch, strings);
}

/* Initialisation */

void Init_amatch()
{
    rb_mAmatch = rb_define_module("Amatch");

    /* Levenshtein */
    rb_cLevenshtein = rb_define_class_under(rb_mAmatch, "Levenshtein", rb_cObject);
    rb_define_alloc_func(rb_cLevenshtein, rb_Levenshtein_s_allocate);
    rb_define_method(rb_cLevenshtein, "initialize", rb_Levenshtein_initialize, 1);
    rb_define_method(rb_cLevenshtein, "pattern", rb_Levenshtein_pattern, 0);
    rb_define_method(rb_cLevenshtein, "pattern=", rb_Levenshtein_pattern_set, 1);
    rb_define_method(rb_cLevenshtein, "substitution", rb_Levenshtein_substitution, 0);
    rb_define_method(rb_cLevenshtein, "substitution=", rb_Levenshtein_substitution_set, 1);
    rb_define_method(rb_cLevenshtein, "deletion", rb_Levenshtein_deletion, 0);
    rb_define_method(rb_cLevenshtein, "deletion=", rb_Levenshtein_deletion_set, 1);
    rb_define_method(rb_cLevenshtein, "insertion", rb_Levenshtein_insertion, 0);
    rb_define_method(rb_cLevenshtein, "insertion=", rb_Levenshtein_insertion_set, 1);
    rb_define_method(rb_cLevenshtein, "reset_weights", rb_Levenshtein_reset_weights, 0);
    rb_define_method(rb_cLevenshtein, "match", rb_Levenshtein_match, 1);
    rb_define_method(rb_cString, "levenshtein_match", rb_str_Levenshtein_match, 1);
    rb_define_method(rb_cLevenshtein, "search", rb_Levenshtein_search, 1);
    rb_define_method(rb_cString, "levenshtein_search", rb_str_Levenshtein_search, 1);

    /* Hamming */
    rb_cHamming = rb_define_class_under(rb_mAmatch, "Hamming", rb_cObject);
    rb_define_alloc_func(rb_cHamming, rb_Hamming_s_allocate);
    rb_define_method(rb_cHamming, "initialize", rb_Hamming_initialize, 1);
    rb_define_method(rb_cHamming, "pattern", rb_General_pattern, 0);
    rb_define_method(rb_cHamming, "pattern=", rb_General_pattern_set, 1);
    rb_define_method(rb_cHamming, "match", rb_Hamming_match, 1);
    rb_define_method(rb_cString, "hamming_match", rb_str_hamming_match, 1);

    /* Pair Distance Metric */
    rb_cPairDistance = rb_define_class_under(rb_mAmatch, "PairDistance", rb_cObject);
    rb_define_alloc_func(rb_cPairDistance, rb_PairDistance_s_allocate);
    rb_define_method(rb_cPairDistance, "initialize", rb_PairDistance_initialize, 1);
    rb_define_method(rb_cPairDistance, "pattern", rb_PairDistance_pattern, 0);
    rb_define_method(rb_cPairDistance, "pattern=", rb_PairDistance_pattern_set, 1);
    rb_define_method(rb_cPairDistance, "match", rb_PairDistance_match, -1);
    rb_define_method(rb_cString, "pair_distance_match", rb_str_pair_distance_match, 1);

    /* Longest Common Subsequence */
    rb_cLongestSubsequence = rb_define_class_under(rb_mAmatch, "LongestSubsequence", rb_cObject);
    rb_define_alloc_func(rb_cLongestSubsequence, rb_LongestSubsequence_s_allocate);
    rb_define_method(rb_cLongestSubsequence, "initialize", rb_LongestSubsequence_initialize, 1);
    rb_define_method(rb_cLongestSubsequence, "pattern", rb_General_pattern, 0);
    rb_define_method(rb_cLongestSubsequence, "pattern=", rb_General_pattern_set, 1);
    rb_define_method(rb_cLongestSubsequence, "match", rb_longest_subsequence_match, 1);
    rb_define_method(rb_cString, "longest_subsequence_match", rb_str_longest_subsequence_match, 1);

    /* Longest Common Substring */
    rb_cLongestSubstring = rb_define_class_under(rb_mAmatch, "LongestSubstring", rb_cObject);
    rb_define_alloc_func(rb_cLongestSubstring, rb_LongestSubstring_s_allocate);
    rb_define_method(rb_cLongestSubstring, "initialize", rb_LongestSubstring_initialize, 1);
    rb_define_method(rb_cLongestSubstring, "pattern", rb_General_pattern, 0);
    rb_define_method(rb_cLongestSubstring, "pattern=", rb_General_pattern_set, 1);
    rb_define_method(rb_cLongestSubstring, "match", rb_LongestSubstring_match_match, 1);
    rb_define_method(rb_cString, "longest_substring_match", rb_str_longest_substring_match, 1);

    id_split = rb_intern("split");
    id_to_f = rb_intern("to_f");
}
    /* vim: set et cin sw=4 ts=4: */
