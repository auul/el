#include <el.h>

void data_echo(struct data value)
{
	switch (value.type) {
	case Nil:
		printf("Nil");
		break;
	case Char:
		printf("'%c", value.c);
		break;
	case Int:
		printf("%li", value.i);
		break;
	case List:
		printf("[");
		for (struct list *list = value.list; list; list = list->cdr) {
			data_echo(list->car);
			if (list->cdr) {
				printf(", ");
			}
		}
		printf("]");
		break;
	default:
		printf("<%p>", value.ptr);
		break;
	}
}

struct data data_ref(struct data value)
{
	switch (value.type) {
	case List:
		ref(value.list);
		break;
	default:
		break;
	}
	return value;
}

struct data data_deref(struct data value)
{
	switch (value.type) {
	case List:
		deref(value.list);
		break;
	default:
		break;
	}
	return value;
}
