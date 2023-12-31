#ifndef EL_H
#define EL_H

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Tagged Data (data.c)

enum data_tag {
	Nil,
	Op,
	Int,
	List,
	Token,
	Map
};

struct range {
	int start;
	int end;
};

struct data {
	enum data_tag tag;
	union {
		unsigned op;
		long int i_num;
		struct range range;
		struct cons *list;
		struct cons *str;
		struct cons *map;
		void *ptr;
	};
};

#define Nil() \
	((struct data) {.tag = Nil, .ptr = NULL})
#define END Nil()
#define Op(value) \
	((struct data) {.tag = Op, .op = value})
#define Int(value) \
	((struct data) {.tag = Int, .i_num = value})
#define List(value) \
	((struct data) {.tag = List, .list = value})
#define Token(value) \
	((struct data) {.tag = Token, .str = value})
#define Map(value) \
	((struct data) {.tag = Map, .map = value})

struct data data_ref(struct data value);
void data_deref(struct data value);
void data_print(struct data value);
bool data_equal(struct data a, struct data b);

// Lists (list.c)

struct cons {
	size_t ref;
	struct data car;
	struct cons *cdr;
};

struct cons *new_cons(struct data car, struct cons *cdr);
struct cons *new_list_v(struct data head, va_list tail);
struct cons *new_list(struct data head, ...);
struct cons *list_ref(struct cons *list);
void list_deref(struct cons *list);
void list_print(struct cons *list);
bool list_equal(struct cons *a, struct cons *b);
struct data list_pop(struct cons **list_p);
struct cons *list_edit(struct cons **head_p, struct cons *end);
struct cons *list_append(struct cons *list, struct data value);
struct cons *list_join(struct cons *a, struct cons *b);

// Strings (str.c)

struct cons *new_str_n(const char *src, size_t len);
struct cons *new_str(const char *src);
#define str_ref(str) list_ref(str)
#define str_deref(str) list_deref(str)
void token_print(struct cons *str);

// Maps (map.c)

struct cons *new_map(struct data key, struct data value);
void map_print(struct cons *map);
struct data map_key(struct cons *map);
struct data map_value(struct cons *map);
struct cons *map_define(struct cons *map, struct data key, struct data value);
struct cons *map_find(struct cons *map, struct data key);
struct data map_lookup(struct cons *map, struct data key);

// Interpreter (interpret.c)

struct state {
	struct cons *dict;
	struct cons *fail;
	struct cons *next;
	struct cons *exec;
	struct cons *stack;
};

enum op_code {
	_NOP,
	_DEF,
	_GETFAIL,
	_SETFAIL,
	_POPNEXT,
	_PUSHNEXT,
	_DROP,
	_DUP,
	_SWAP,
	_I,
	_FORK,
	_TEST,
	_NOT,
	_LOOP,
	_GEN,
	_INVERT,
	_CONTAIN,
	_UNIT,
	_POP,
	_PUSH,
	_CAT,
	_CHOOSE,
	_ADD,
	_SUBTRACT,
	_MULTIPLY,
	_DIVIDE,
	_EQUAL,
};

void state_print(struct state state);
void op_print(enum op_code op);
struct state eval(struct state state);
struct cons *generate(struct cons *dict, struct cons *exec, struct cons *stack);

#endif
