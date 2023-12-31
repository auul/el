#include <el.h>

struct cons *new_cons(struct data car, struct cons *cdr)
{
	struct cons *cons = malloc(sizeof(struct cons));
	if (!cons) {
		perror(strerror(errno));
		exit(1);
	}

	cons->ref = 1;
	cons->car = car;
	cons->cdr = cdr;

	return cons;
}

struct cons *new_list_v(struct data head, va_list tail)
{
	if (head.tag == Nil) {
		return NULL;
	}

	struct cons *list = new_cons(head, NULL);
	struct cons *node = list;
	head = va_arg(tail, struct data);

	while (head.tag != Nil) {
		node->cdr = new_cons(head, NULL);
		node = node->cdr;
		head = va_arg(tail, struct data);
	}

	return list;
}

struct cons *new_list(struct data head, ...)
{
	va_list tail;
	va_start(tail, head);
	struct cons *list = new_list_v(head, tail);
	va_end(tail);
	return list;
}

struct cons *list_ref(struct cons *list)
{
	if (list) {
		list->ref++;
	}
	return list;
}

void list_deref(struct cons *list)
{
	while (list) {
		list->ref--;
		if (list->ref) {
			return;
		}

		data_deref(list->car);
		struct cons *cdr = list->cdr;
		free(list);
		list = cdr;
	}
}

void list_print(struct cons *list)
{
	while (list) {
		data_print(list->car);
		list = list->cdr;
		if (list) {
			printf(" ");
		}
	}
}

bool list_equal(struct cons *a, struct cons *b)
{
	while (a) {
		if (a == b) {
			return true;
		} else if (!b || !data_equal(a->car, b->car)) {
			return false;
		}

		a = a->cdr;
		b = b->cdr;
	}
	return !b;
}

struct data list_pop(struct cons **list_p)
{
	struct cons *list = *list_p;
	if (!list) {
		return Nil();
	}

	struct data value = data_ref(list->car);
	*list_p = list_ref(list->cdr);
	list_deref(list);

	return value;
}

struct cons *list_edit(struct cons **head_p, struct cons *end)
{
	struct cons *node = *head_p;
	if (!node) {
		return NULL;
	}

	if (node->ref > 1) {
		list_deref(node);
		node = new_cons(data_ref(node->car), list_ref(node->cdr));
		if (*head_p == end) {
			*head_p = node;
			return node;
		}
		*head_p = node;
	} else if (node == end) {
		return node;
	}

	while (node->cdr != end) {
		if (node->cdr->ref > 1) {
			list_deref(node->cdr);
			node->cdr = new_cons(data_ref(node->cdr->car), list_ref(node->cdr->cdr));
		}
		node = node->cdr;
	}

	if (end) {
		if (end->ref > 1) {
			list_deref(end);
			node->cdr = new_cons(data_ref(end->car), list_ref(end->cdr));
		}
		return node->cdr;
	}

	return node;
}

struct cons *list_append(struct cons *list, struct data value)
{
	if (!list) {
		return new_cons(value, NULL);
	}

	list_edit(&list, NULL)->cdr = new_cons(value, NULL);
	return list;
}

struct cons *list_join(struct cons *a, struct cons *b)
{
	if (!a) {
		return b;
	} else if (!b) {
		return a;
	}

	list_edit(&a, NULL)->cdr = b;
	return a;
}
