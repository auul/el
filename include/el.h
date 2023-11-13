#ifndef EL_H
#define EL_H

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Flexible Data (data.c)

enum type {
	Nil,
	Char,
	Int,
	List
};

struct data {
	enum type type;
	union {
		char c;
		long int i;
		struct list *list;
		void *ptr;
	};
};

#define Nil() \
	((struct data){.type = Nil, .ptr = NULL})
#define Char(value) \
	((struct data){.type = Char, .c = value})
#define Int(value) \
	((struct data){.type = Int, .i = value})
#define List(value) \
	((struct data){.type = List, .list = value})

void data_echo(struct data value);
struct data data_ref(struct data value);
struct data data_deref(struct data value);

// Memory Management (gc.c)

struct tag {
	struct tag *prev;
	size_t ref;
	size_t size;
	enum type type;
	unsigned char ptr[];
};

#define get_tag(p) \
	((struct tag *)((unsigned char *)(p) - offsetof(struct tag, ptr)))

void *alloc(enum type type, size_t size);
void *ref(void *ptr);
void *deref(void *ptr);
void dump_garbage(struct tag *end_at);

// Lists (list.c)

struct list {
	struct data car;
	struct list *cdr;
};

struct list *list_push(struct list *cdr, struct data car);
struct list *list_edit(struct list **head_p, struct list *end);

// Bytecode (bc.c)

enum error {
	Success,
	Failure,
	ErrAllocate,
	ErrTypeMismatch
};

struct retval {
	enum error err;
	struct list *stack;
};

#define Retval(e, s) \
	((struct retval){.err = e, .stack = s})

struct retval bc_match_char(struct data value, struct list *stack);

#endif
