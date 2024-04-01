#ifndef EL_H
#define EL_H

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef enum data_type el_type;
typedef struct data el_data;
typedef uint32_t el_char;
typedef int32_t el_int;
typedef struct word el_word;
typedef uint32_t el_ref;
typedef struct cons el_list;
typedef struct cons el_choice;
typedef struct cons el_fn;
typedef enum built_in el_built_in;
typedef struct dict el_dict;

// Data Types (data.c)

enum data_type {
	Nil,
	Any,
	Char,
	Int,
	Word,
	List,
	Choice,
	Fn,
	BuiltIn,
};

struct data {
	el_type type;
	union {
		void *ptr;
		el_type any;
		el_char c[2];
		el_int i[2];
		el_word *word;
		el_list *list;
		el_choice *choice;
		el_fn *fn;
		unsigned built_in;
	};
};

#define Nil() \
	((el_data) {.type = Nil, .ptr = NULL})
#define Any(value) \
	((el_data) {.type = Any, .any = value})
#define Char(value) \
	((el_data) {.type = Char, .c = {value, value}})
#define CharRange(from, to) \
	((el_data) {.type = Char, .c = {from, to}})
#define Int(value) \
	((el_data) {.type = Int, .i = {value, value}})
#define IntRange(from, to) \
	((el_data) {.type = Int, .i = {from, to}})
#define Word(ptr) \
	((el_data) {.type = Word, .word = ptr})
#define List(ptr) \
	((el_data) {.type = List, .list = ptr})
#define Choice(ptr) \
	((el_data) {.type = Choice, .choice = ptr})
#define Fn(ptr) \
	((el_data) {.type = Fn, .fn = ptr})
#define BuiltIn(value) \
	((el_data) {.type = BuiltIn, .built_in = value})

void data_print_struct_f(FILE *f, el_data data);
void data_print_struct(el_data data);
void data_print(el_data data);
el_data data_ref(el_data data);
void data_deref(el_data data);
int data_compare(el_data a, el_data b);
bool data_equal(el_data a, el_data b);
el_data data_merge(el_data a, el_data b);
el_data data_unify(el_dict **env_p, el_data a, el_data b);
el_data data_choose(el_data *data_p);
el_data eval(el_dict **env_p, el_data ctrl);

// Characters (char.c)

unsigned utf8_char_size(const char *src);
size_t utf8_len(const char *src);
size_t utf8_size(const char *src, size_t len);
el_char char_read(const char **src_p);
char *char_write(char *dest, el_char c);
void char_print_raw_f(FILE *f, el_char c);
void char_print_esc_f(FILE *f, el_char c);
void char_print_struct_f(FILE *f, el_char c[2]);
int char_compare(el_char a[2], el_char b[2]);
bool char_equal(el_char a[2], el_char b[2]);
el_data char_merge(el_char a[2], el_char b[2]);
el_data char_unify_char(el_char a[2], el_char b[2]);
el_data char_choose(el_data *data_p);

// Integers (int.c)

void int_print_struct_f(FILE *f, el_int i[2]);
int int_compare(el_int a[2], el_int b[2]);
bool int_equal(el_int a[2], el_int b[2]);
el_data int_merge(el_int a[2], el_int b[2]);
el_data int_unify_int(el_int a[2], el_int b[2]);
el_data int_choose(el_data *data_p);

// Word (word.c)

struct word {
	el_ref ref;
	char str[];
};

el_word *new_word_n(const char *src, size_t bytes);
el_word *new_word(const char *src);
void word_print_struct_f(FILE *f, el_word *word);
el_word *word_ref(el_word *word);
void word_deref(el_word *word);
int word_compare(el_word *a, el_word *b);
bool word_equal(el_word *a, el_word *b);

// Lists (list.c)

struct cons {
	el_ref ref;
	el_data car;
	struct cons *cdr;
};

size_t debug_get_num_cons(void);
struct cons *new_cons(el_data car, struct cons *cdr);
el_list *new_list_v(el_data car, va_list args);
el_list *new_list(el_data car, ...);
el_list *new_str_n(const char *src, size_t bytes);
el_list *new_str(const char *src);
void list_print_raw_f(FILE *f, el_list *list);
void list_print_struct_f(FILE *f, el_list *list);
el_list *list_ref(el_list *list);
void list_deref(el_list *list);
int list_compare(el_list *a, el_list *b);
bool list_equal(el_list *a, el_list *b);
el_data list_pop(el_list **list_p);
el_list *list_reverse(el_list *list);
el_data list_unify_list(el_dict **env_p, el_list *a, el_list *b);
size_t list_len(el_list *list);
el_list *list_edit(el_list **list_p, el_list *node);
el_list *list_join(el_list *a, el_list *b);
el_list *list_append(el_list *list, el_data value);
el_list *list_prev(el_list *list, el_list *node);
el_list *list_node(el_list *list, size_t index);
el_list *list_delete(el_list *list, el_list *node);
el_list *list_fork(el_list **b_list_p, el_list **b_node_p, el_list **a_list_p, el_list *node);
el_data list_choose(el_data *data_p);
el_data list_eval(el_dict **env_p, el_list *list);

// Choices (choice.c)

#define new_choice_v(car, args) new_list_v(car, args)
#define new_choice(...) new_list(__VA_ARGS__)
void choice_print_struct_f(FILE *f, el_choice *choice);
#define choice_ref(choice) list_ref(choice)
#define choice_deref(choice) list_deref(choice)
#define choice_compare(a, b) list_compare(a, b)
#define choice_equal(a, b) list_equal(a, b)
#define choice_pop(choice_p) list_pop(choice_p)
#define choice_edit(choice_p, node) list_edit(choice_p, node)
#define choice_join(a, b) list_join(a, b)
#define choice_delete(choice, node) list_delete(choice, node)
el_data choice_simplify(el_choice *choice);
el_data choice_unify(el_dict **env_p, el_choice *a, el_data b);
el_data choice_choose(el_data *data_p);
el_data choice_rejoin(el_data ctrl, el_data alt);
el_data choice_eval(el_dict **env_p, el_choice *choice);

// Functions (fn.c)

enum built_in {
	FnUnify,
	FnJoin,
	FnAdd,
};

#define new_fn(...) new_list(__VA_ARGS__)
#define new_built_in(built_in, ...) new_list(BuiltIn(built_in), __VA_ARGS__)
void fn_print_struct_f(FILE *f, el_fn *fn);
void fn_built_in_print_f(FILE *f, el_built_in built_in);
#define fn_ref(fn) list_ref(fn)
#define fn_deref(fn) list_deref(fn)
#define fn_compare(a, b) list_compare(a, b)
#define fn_equal(a, b) list_equal(a, b)
#define fn_pop(fn_p) list_pop(fn_p)
#define fn_edit(fn_p, node) list_edit(fn_p, node)
el_data fn_eval(el_dict **env_p, el_fn *fn);

// Dictionary (dict.c)

struct dict {
	el_ref ref;
	el_word *key;
	el_data value;
	struct dict *left;
	struct dict *right;
};

el_dict *new_dict_entry(el_word *key, el_data value, el_dict *left, el_dict *right);
void dict_print_struct_f(FILE *f, el_dict *dict);
void dict_print_raw_f(FILE *f, el_dict *dict);
void dict_print_f(FILE *f, el_dict *dict);
void dict_print(el_dict *dict);
el_dict *dict_ref(el_dict *dict);
void dict_deref(el_dict *dict);
el_dict *dict_anchor(el_dict *dict, el_word *key);
el_dict *dict_entry(el_dict *dict, el_word *key);
el_data dict_lookup(el_dict *dict, el_word *key);
el_dict *dict_edit(el_dict **dict_p, el_dict *entry);
el_dict *dict_define(el_dict *dict, el_word *key, el_data value);

#endif
