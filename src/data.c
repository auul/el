#include <el.h>

struct data data_ref(struct data value)
{
	switch (value.tag) {
	case List:
	case Token:
	case Map:
		list_ref(value.list);
		break;
	default:
		break;
	}

	return value;
}

void data_deref(struct data value)
{
	switch (value.tag) {
	case List:
	case Token:
	case Map:
		list_deref(value.list);
		break;
	default:
		break;
	}
}

void data_print(struct data value)
{
	switch (value.tag) {
	case Nil:
		printf("nil");
		break;
	case Op:
		op_print(value.op);
		break;
	case Int:
		printf("%li", value.i_num);
		break;
	case List:
		printf("[");
		list_print(value.list);
		printf("]");
		break;
	case Token:
		token_print(value.str);
		break;
	case Map:
		if (value.map) {
			data_print(value.map->cdr->car);
			printf(":");
			data_print(value.map->car);
		} else {
			printf("nil");
		}
		break;
	default:
		printf("<%p>", value.ptr);
		break;
	}
}

bool data_equal(struct data a, struct data b)
{
	if (a.tag != b.tag) {
		return false;
	} else if (a.ptr == b.ptr) {
		return true;
	}

	switch (a.tag) {
	case Op:
		return a.op == b.op;
	case Int:
		return a.i_num == b.i_num;
	case List:
	case Token:
	case Map:
		return list_equal(a.list, b.list);
	default:
		return false;
	}
}
