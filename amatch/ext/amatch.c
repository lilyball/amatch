#include "ruby.h"

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
 * Edit distances are calculated here
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
calculate_distance (self, string, mode)
    VALUE self;
	VALUE string;
	char mode;
{
	VALUE pattern, tmp;
	static VALUE result;
	int pattern_len, string_len;
	char *pattern_ptr, *string_ptr;
	vector *v[2];
	int weight, sw, dw, iw, i, j, tmpi;
	int c = 0, p = 1;

	Check_Type(string, T_STRING);
	string_ptr = RSTRING(string)->ptr;
	string_len = RSTRING(string)->len;

	pattern = rb_iv_get(self, "@pattern");
	Check_Type(pattern, T_STRING);
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
			rb_raise(rb_eFatal, "unknown mode in calculate_distance");
	}

	v[1] = vector_new(string_len);
	for (i = 1; i <= pattern_len; i++) {
		c = i % 2;				/* current row */
		p = (i - 1) % 2;		/* previous row */
		v[c]->ptr[0] = i * dw;	/* first column */
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
				(string_len < pattern_len ? -1 : 1)	 *
				vector_last(v[c]) / pattern_len);
			break;
		default:
			rb_raise(rb_eFatal, "unknown mode in calculate_distance");
	}
	vector_destroy(v[0]);
	vector_destroy(v[1]);
	return result;
}

static VALUE
handle_strings(self, strings, mode)
	VALUE self;
	VALUE strings;
	char mode;
{
	if (TYPE(strings) == T_ARRAY) {
		int i;
		VALUE result = rb_ary_new2(RARRAY(strings)->len);
		for (i = 0; i < RARRAY(strings)->len; i++) {
			VALUE string = rb_ary_entry(strings, i);
			if (TYPE(string) != T_STRING) {
				rb_raise(rb_eTypeError,
					"array has to contain only strings (%s given)",
					NIL_P(string) ? "NilClass" :
									rb_class2name(CLASS_OF(string)));
			}
			rb_ary_push(result, calculate_distance(self, string, mode));
		}
		return result;
	} else if (TYPE(strings) == T_STRING) {
		return calculate_distance(self, strings, mode);
	} else {
		rb_raise(rb_eTypeError,
			"value of strings needs to be string or array (%s given)",
			NIL_P(strings) ? "NilClass" : rb_class2name(CLASS_OF(strings)));
	}
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
rb_amatch_initialize(argc, argv, self)
    int argc;
    VALUE* argv;
    VALUE self;
{
    VALUE pattern;

    rb_scan_args(argc, argv, "01", &pattern);
	Check_Type(pattern, T_STRING);
	rb_iv_set(self, "@pattern", pattern);

	rb_amatch_resetw(self);

    return self;
}

static VALUE
rb_amatch_pattern_is(self, pattern)
    VALUE self;
	VALUE pattern;
{
	Check_Type(pattern, T_STRING);
	rb_iv_set(self, "@pattern", pattern);

	return pattern;
}


static VALUE
rb_amatch_match(self, strings)
	VALUE self;
	VALUE strings; 
{
	return handle_strings(self, strings, MATCH);
}

static VALUE
rb_amatch_matchr(self, strings)
	VALUE self;
	VALUE strings;
{
	return handle_strings(self, strings, MATCHR);
}

static VALUE
rb_amatch_compare(self, strings)
	VALUE self;
	VALUE strings;
{
	return handle_strings(self, strings, COMPARE);
}

static VALUE
rb_amatch_comparer(self, strings)
	VALUE self;
	VALUE strings;
{
	return handle_strings(self, strings, COMPARER);
}


static VALUE
rb_amatch_search(self, strings)
	VALUE self;
	VALUE strings;
{
	return handle_strings(self, strings, SEARCH);
}

static VALUE
rb_amatch_searchr(self, strings)
	VALUE self;
	VALUE strings;
{
	return handle_strings(self, strings, SEARCHR);
}

void
Init_amatch()
{
	cAmatch = rb_define_class("Amatch", rb_cObject);
    rb_define_method(cAmatch, "initialize", rb_amatch_initialize, -1);

	rb_define_attr(cAmatch, "debug", 1, 1);
	rb_define_attr(cAmatch, "subw", 1, 1);
	rb_define_attr(cAmatch, "delw", 1, 1);
	rb_define_attr(cAmatch, "insw", 1, 1);
    rb_define_method(cAmatch, "resetw", rb_amatch_resetw, 0);

    rb_define_method(cAmatch, "pattern=", rb_amatch_pattern_is, 1);
	rb_define_attr(cAmatch, "pattern", 1, 0);

    rb_define_method(cAmatch, "match", rb_amatch_match, 1);
    rb_define_method(cAmatch, "matchr", rb_amatch_matchr, 1);
    rb_define_method(cAmatch, "compare", rb_amatch_compare, 1);
    rb_define_method(cAmatch, "comparer", rb_amatch_comparer, 1);
    rb_define_method(cAmatch, "search", rb_amatch_search, 1);
    rb_define_method(cAmatch, "searchr", rb_amatch_searchr, 1);
}
	/* vim: set cin sw=4 ts=4: */
