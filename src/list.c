#include <el.h>

size_t num_cons = 0;

size_t debug_get_num_cons(void)
{
	return num_cons;
}

struct cons *new_cons(el_data car, struct cons *cdr)
{
	struct cons *cons = malloc(sizeof(struct cons));
	if (!cons) {
		// TODO ERROR ALLOC
		perror(strerror(errno));
		exit(1);
	}

	cons->ref = 1;
	cons->car = car;
	cons->cdr = cdr;

	// TODO DEBUG
	num_cons++;

	return cons;
}

el_list *new_list_v(el_data car, va_list args)
{
	if (car.type == Nil) {
		return NULL;
	}

	el_list *list = new_cons(car, NULL);
	el_list *node = list;
	car = va_arg(args, el_data);

	while (car.type != Nil) {
		node->cdr = new_cons(car, NULL);
		node = node->cdr;
		car = va_arg(args, el_data);
	}

	return list;
}

el_list *new_list(el_data car, ...)
{
	va_list args;
	va_start(args, car);
	el_list *list = new_list_v(car, args);
	va_end(args);
	return list;
}

el_list *new_str_n(const char *src, size_t bytes)
{
	if (!src || !bytes) {
		return NULL;
	}

	const char *end = src + bytes;
	el_char c = char_read(&src);
	if (!c) {
		return NULL;
	}

	el_list *str = new_cons(Char(c), NULL);
	el_list *node = str;

	while (src < end) {
		c = char_read(&src);
		if (!c) {
			list_deref(str);
			return NULL;
		}

		node->cdr = new_cons(Char(c), NULL);
		node = node->cdr;
	}

	if (src != end) {
		// TODO ERROR INVALID UTF8 SEQUENCE
		list_deref(str);
		return NULL;
	}

	return str;
}

el_list *new_str(const char *src)
{
	return new_str_n(src, strlen(src));
}

bool list_is_str(el_list *list)
{
	while (list) {
		if (list->car.type != Char || list->car.c[0] != list->car.c[1]) {
			return false;
		}
		list = list->cdr;
	}

	return true;
}

void str_print_struct_f(FILE *f, el_list *str)
{
	fprintf(f, "\"");
	while (str) {
		if (str->car.c[0] == '"') {
			fprintf(f, "\\\"");
		} else {
			char_print_raw_f(f, str->car.c[0]);
		}
		str = str->cdr;
	}
	fprintf(f, "\"");
}

void list_print_raw_f(FILE *f, el_list *list)
{
	while (list) {
		data_print(list->car);
		list = list->cdr;
		if (list) {
			fprintf(f, ", ");
		}
	}
}

void list_print_struct_f(FILE *f, el_list *list)
{
	if (list_is_str(list)) {
		str_print_struct_f(f, list);
		return;
	}

	fprintf(f, "[");
	list_print_raw_f(f, list);
	fprintf(f, "]");
}

el_list *list_ref(el_list *list)
{
	if (list) {
		list->ref++;
	}
	return list;
}

void list_deref(el_list *list)
{
	while (list) {
		list->ref--;
		if (list->ref) {
			return;
		}

		data_deref(list->car);
		el_list *cdr = list->cdr;
		free(list);

		// TODO DEBUG
		num_cons--;

		list = cdr;
	}
}

int list_compare(el_list *a, el_list *b)
{
	while (a) {
		if (a == b) {
			return 0;
		} else if (!b) {
			return -1;
		}

		int cmp = data_compare(a->car, b->car);
		if (cmp) {
			return cmp;
		}

		a = a->cdr;
		b = b->cdr;
	}

	return b ? 1 : 0;
}

bool list_equal(el_list *a, el_list *b)
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

el_data list_pop(el_list **list_p)
{
	assert(list_p);

	el_list *list = *list_p;
	el_data car = data_ref(list->car);
	*list_p = list_ref(list->cdr);

	list_deref(list);
	return car;
}

el_list *list_reverse(el_list *list)
{
	el_list *out = NULL;
	while (list) {
		out = new_cons(list_pop(&list), out);
	}
	return out;
}

el_data list_unify_list(el_dict **env_p, el_list *a, el_list *b)
{
	el_list *out = NULL;

	while (a) {
		el_data a_car = list_pop(&a);
		el_data b_car = list_pop(&b);
		el_data value = data_unify(env_p, a_car, b_car);

		if (value.type == Nil) {
			list_deref(a);
			list_deref(b);
			list_deref(out);
			return Nil();
		}

		out = new_cons(value, out);
	}

	if (b) {
		list_deref(b);
		list_deref(out);
		return Nil();
	}

	return List(list_reverse(out));
}

size_t list_len(el_list *list)
{
	size_t len = 0;
	while (list) {
		len++;
		list = list->cdr;
	}
	return len;
}

el_list *list_edit(el_list **list_p, el_list *node)
{
	assert(list_p);

	el_list *list = *list_p;
	if (!list) {
		return NULL;
	} else if (list->ref > 1) {
		list_deref(list);
		list = new_cons(data_ref(list->car), list_ref(list->cdr));
		if (*list_p == node) {
			*list_p = list;
			return list;
		}
		*list_p = list;
	} else if (list == node) {
		return list;
	}

	while (list->cdr != node) {
		if (list->cdr->ref > 1) {
			list_deref(list->cdr);
			list->cdr = new_cons(data_ref(list->cdr->car), list_ref(list->cdr->cdr));
		}
		list = list->cdr;
	}

	if (node) {
		if (node->ref > 1) {
			list_deref(node);
			list->cdr = new_cons(data_ref(node->car), list_ref(node->cdr));
			return list->cdr;
		}
		return node;
	}
	return list;
}

el_list *list_join(el_list *a, el_list *b)
{
	if (!a) {
		if (!b) {
			return NULL;
		}
		return b;
	} else if (!b) {
		return a;
	}

	list_edit(&a, NULL)->cdr = b;
	return a;
}

el_list *list_append(el_list *list, el_data value)
{
	return list_join(list, new_cons(value, NULL));
}

el_list *list_prev(el_list *list, el_list *node)
{
	if (!list || list == node) {
		return NULL;
	}

	while (list->cdr != node) {
		list = list->cdr;
	}

	return list;
}

el_list *list_node(el_list *list, size_t index)
{
	if (!list) {
		return NULL;
	}

	while (list && index) {
		list = list->cdr;
		index--;
	}
	return list;
}

el_list *list_delete(el_list *list, el_list *node)
{
	if (!list || !node) {
		return list;
	} else if (list == node) {
		data_deref(list_pop(&list));
		return list;
	} else if (list->ref > 1) {
		list_deref(list);
		list = new_cons(data_ref(list->car), list_ref(list->cdr));
	}

	el_list *prev = list;

	while (prev->cdr != node) {
		if (prev->cdr->ref > 1) {
			list_deref(prev->cdr);
			prev->cdr = new_cons(data_ref(prev->cdr->car), list_ref(prev->cdr->cdr));
		}
		prev = prev->cdr;
	}

	prev->cdr = list_ref(node->cdr);
	list_deref(node);

	return list;
}

el_list *list_fork(el_list **b_list_p, el_list **b_node_p, el_list **a_list_p, el_list *node)
{
	*b_list_p = list_ref(*a_list_p);
	*b_node_p = list_edit(b_list_p, node);
	return list_edit(a_list_p, node);
}

el_data list_choose(el_data *data_p)
{
	el_data data = *data_p;
	*data_p = Nil();

	for (el_list *node = data.list; node; node = node->cdr) {
		el_data car = data_ref(node->car);
		el_data pick = data_choose(&car);
		if (car.type == Nil) {
			data_deref(pick);
		} else {
			el_data retval = data_ref(data);
			el_list *data_node;
			node = list_fork(&data.list, &data_node, &retval.list, node);

			data_deref(data_node->car);
			data_node->car = car;
			*data_p = data;

			data_deref(node->car);
			node->car = pick;
			return retval;
		}
	}

	return data;
}

el_data list_eval(el_dict **env_p, el_list *list)
{
	if (!list) {
		return List(NULL);
	}

	list_edit(&list, NULL);
	for (el_list *node = list; node; node = node->cdr) {
		node->car = eval(env_p, node->car);
		if (node->car.type == Nil) {
			list_deref(list);
			return Nil();
		}
	}

	return List(list);
}
