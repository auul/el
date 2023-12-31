#include <el.h>

void state_print(struct state state)
{
	printf("Dict: ");
	list_print(state.dict);
	printf("\n");

	printf("Fail: ");
	list_print(state.fail);
	printf("\n");

	printf("Next: ");
	list_print(state.next);
	printf("\n");

	printf("Exec: ");
	list_print(state.exec);
	printf("\n");

	printf("Stack: ");
	list_print(state.stack);
	printf("\n\n");
}

void op_print(enum op_code op)
{
	switch (op) {
	case _NOP:
		printf("nop");
		break;
	case _DEF:
		printf("def");
		break;
	case _GETFAIL:
		printf("<<fail");
		break;
	case _SETFAIL:
		printf(">>fail");
		break;
	case _POPNEXT:
		printf("<next");
		break;
	case _PUSHNEXT:
		printf(">next");
		break;
	case _DUP:
		printf("dup");
		break;
	case _DROP:
		printf("drop");
		break;
	case _SWAP:
		printf("swap");
		break;
	case _I:
		printf("i");
		break;
	case _FORK:
		printf("fork");
		break;
	case _TEST:
		printf("test");
		break;
	case _NOT:
		printf("not");
		break;
	case _LOOP:
		printf("loop");
		break;
	case _GEN:
		printf("gen");
		break;
	case _INVERT:
		printf("invert");
		break;
	case _CONTAIN:
		printf("contain");
		break;
	case _UNIT:
		printf("unit");
		break;
	case _POP:
		printf("pop");
		break;
	case _PUSH:
		printf("push");
		break;
	case _CAT:
		printf("cat");
		break;
	case _CHOOSE:
		printf("choose");
		break;
	case _EQUAL:
		printf("=");
		break;
	case _ADD:
		printf("+");
		break;
	case _SUBTRACT:
		printf("-");
		break;
	case _MULTIPLY:
		printf("*");
		break;
	case _DIVIDE:
		printf("/");
		break;
	default:
		printf("op(%u)", op);
		break;
	}
}

struct state fail_state(struct state state)
{
	list_deref(state.next);
	list_deref(state.exec);

	if (!state.fail) {
		list_deref(state.stack);
		state.next = NULL;
		state.exec = NULL;
		state.stack = NULL;
		return state;
	}

	struct data value = list_pop(&state.fail);
	if (value.tag == List) {
		state.next = value.list;
	} else {
		state.next = new_cons(value, NULL);
	}

	state.exec = NULL;
	return state;
}

void push_fail(struct state *state, struct cons *next, struct cons *exec, struct cons *stack)
{
	exec = new_cons(List(stack), new_cons(Op(_INVERT), new_cons(Op(_DROP), exec)));
	next = new_cons(List(exec), next);
	state->fail = new_cons(List(next), state->fail);
}

struct state op_eval(enum op_code op, struct state state)
{
	struct data a, b;

	switch (op) {
	case _DEF:
		if (!state.stack || !state.stack->cdr || state.stack->car.tag != List || !state.stack->car.list || state.stack->car.list->car.tag != Token || state.stack->car.list->cdr) {
			return fail_state(state);
		}
		a = list_pop(&state.stack);
		b = list_pop(&a.list);
		a = list_pop(&state.stack);

		state.dict = map_define(state.dict, b, a);
		break;
	case _GETFAIL:
		state.stack = new_cons(List(state.fail), state.stack);
		state.fail = NULL;
		break;
	case _SETFAIL:
		if (!state.stack) {
			return fail_state(state);
		}

		a = list_pop(&state.stack);
		if (a.tag != List) {
			a = List(new_cons(a, NULL));
		}

		state.fail = a.list;
		break;
	case _POPNEXT:
		state.stack = new_cons(List(state.next), state.stack);
		break;
	case _PUSHNEXT:
		if (!state.stack) {
			return fail_state(state);
		}

		a = list_pop(&state.stack);
		if (a.tag != List) {
			a = List(new_cons(a, NULL));
		}

		state.next = new_cons(a, state.next);
		break;
	case _DUP:
		if (!state.stack) {
			return fail_state(state);
		}
		a = list_pop(&state.stack);
		state.stack = new_cons(a, new_cons(data_ref(a), state.stack));
		break;
	case _DROP:
		if (!state.stack) {
			return fail_state(state);
		}
		data_deref(list_pop(&state.stack));
		break;
	case _SWAP:
		if (!state.stack || !state.stack->cdr) {
			return fail_state(state);
		}
		a = list_pop(&state.stack);
		b = list_pop(&state.stack);
		state.stack = new_cons(b, new_cons(a, state.stack));
		break;
	case _I:
		if (!state.stack) {
			return fail_state(state);
		} else if (state.stack->car.tag == Op) {
			return op_eval(list_pop(&state.stack).op, state);
		} else if (state.stack->car.tag != List) {
			return state;
		}
		state.next = new_cons(List(state.exec), state.next);
		state.exec = list_pop(&state.stack).list;
		break;
	case _FORK:
		if (!state.stack || state.stack->car.tag != List) {
			return fail_state(state);
		}
		a = list_pop(&state.stack);
		push_fail(&state, list_ref(state.next), state.exec, list_ref(state.stack));
		state.exec = a.list;
		break;
	case _TEST:
		if (!state.stack || state.stack->car.tag != List) {
			return fail_state(state);
		}
		a = list_pop(&state.stack);
		state.next = new_cons(List(new_cons(List(list_ref(state.stack)), new_cons(Op(_INVERT), new_cons(Op(_DROP), state.exec)))), state.next);
		state.exec = a.list;
		break;
	case _LOOP:
		if (!state.stack || state.stack->car.tag != List) {
			return fail_state(state);
		}
		a = list_pop(&state.stack);
		push_fail(&state, list_ref(state.next), list_ref(state.exec), list_ref(state.stack));
		state.next = new_cons(List(new_cons(data_ref(a), new_cons(Op(_LOOP), state.exec))), state.next);
		state.exec = a.list;
		break;
	case _GEN:
		if (!state.stack) {
			return fail_state(state);
		} else if (state.stack->car.tag != List) {
			state.stack = new_cons(List(list_ref(state.stack)), state.stack);
			return state;
		} else if (!state.stack->car.list) {
			return state;
		}
		a = list_pop(&state.stack);
		state.stack = generate(state.dict, a.list, state.stack);
		break;
	case _INVERT:
		if (!state.stack || state.stack->car.tag != List) {
			return fail_state(state);
		}

		a = list_pop(&state.stack);
		state.stack = new_cons(List(state.stack), a.list);
		break;
	case _CONTAIN:
		if (!state.stack) {
			return fail_state(state);
		}
		a = list_pop(&state.stack);
		state.next = new_cons(List(new_cons(List(state.stack), new_cons(Op(_INVERT), state.exec))), state.next);
		state.stack = NULL;
		state.exec = a.list;
		break;
	case _UNIT:
		if (!state.stack) {
			return fail_state(state);
		}
		a = list_pop(&state.stack);
		state.stack = new_cons(List(new_cons(a, NULL)), state.stack);
		break;
	case _POP:
		if (!state.stack || state.stack->car.tag != List || !state.stack->car.list) {
			return fail_state(state);
		}

		a = list_pop(&state.stack);
		b = list_pop(&a.list);
		state.stack = new_cons(b, new_cons(a, state.stack));
		break;
	case _PUSH:
		if (!state.stack || !state.stack->cdr) {
			return fail_state(state);
		}

		a = list_pop(&state.stack);
		b = list_pop(&state.stack);
		if (b.tag == List) {
			b.list = new_cons(a, b.list);
		} else {
			b = List(new_cons(a, new_cons(b, NULL)));
		}
		state.stack = new_cons(b, state.stack);
		break;
	case _CAT:
		if (!state.stack || !state.stack->cdr) {
			return fail_state(state);
		}

		a = list_pop(&state.stack);
		b = list_pop(&state.stack);
		if (b.tag == List) {
			if (a.tag == List) {
				b.list = list_join(b.list, a.list);
			} else {
				b.list = list_append(b.list, a);
			}
		} else if (a.tag == List) {
			b = List(new_cons(b, a.list));
		} else {
			b = List(new_cons(b, new_cons(a, NULL)));
		}
		state.stack = new_cons(b, state.stack);
		break;
	case _CHOOSE:
		if (!state.stack) {
			return fail_state(state);
		} else if (state.stack->car.tag != List) {
			return state;
		} else if (!state.stack->car.list) {
			return fail_state(state);
		}

		a = list_pop(&state.stack);
		b = list_pop(&a.list);
		push_fail(&state, list_ref(state.next), new_cons(a, new_cons(Op(_CHOOSE), list_ref(state.exec))), list_ref(state.stack));
		state.stack = new_cons(b, state.stack);
		break;
	case _EQUAL:
		if (!state.stack || !state.stack->cdr) {
			return fail_state(state);
		}
		a = list_pop(&state.stack);
		b = list_pop(&state.stack);
		if (!data_equal(a, b)) {
			data_deref(a);
			data_deref(b);
			return fail_state(state);
		}
		data_deref(a);
		data_deref(b);
		break;
	case _ADD:
		if (!state.stack || !state.stack->cdr || state.stack->car.tag != Int || state.stack->cdr->car.tag != Int) {
			return fail_state(state);
		}
		a = list_pop(&state.stack);
		b = list_pop(&state.stack);
		state.stack = new_cons(Int(b.i_num + a.i_num), state.stack);
		break;
	case _SUBTRACT:
		if (!state.stack || !state.stack->cdr || state.stack->car.tag != Int || state.stack->cdr->car.tag != Int) {
			return fail_state(state);
		}
		a = list_pop(&state.stack);
		b = list_pop(&state.stack);
		state.stack = new_cons(Int(b.i_num - a.i_num), state.stack);
		break;
	case _MULTIPLY:
		if (!state.stack || !state.stack->cdr || state.stack->car.tag != Int || state.stack->cdr->car.tag != Int) {
			return fail_state(state);
		}
		a = list_pop(&state.stack);
		b = list_pop(&state.stack);
		state.stack = new_cons(Int(b.i_num * a.i_num), state.stack);
		break;
	case _DIVIDE:
		if (!state.stack || !state.stack->cdr || state.stack->car.tag != Int || state.stack->cdr->car.tag != Int) {
			return fail_state(state);
		}
		a = list_pop(&state.stack);
		b = list_pop(&state.stack);
		state.stack = new_cons(Int(b.i_num / a.i_num), state.stack);
		break;
	default:
		break;
	}

	return state;
}

struct state eval(struct state state)
{
	struct data key, value;
	do {
		while (state.exec) {
			value = list_pop(&state.exec);
			switch (value.tag) {
			case Op:
				state = op_eval(value.op, state);
				break;
			case Token:
			AnotherToken:
				key = value;
				value = data_ref(map_lookup(state.dict, key));
				if (value.tag == Token) {
					data_deref(key);
					goto AnotherToken;
				}

				switch (value.tag) {
				case Nil:
					state.stack = new_cons(key, state.stack);
					break;
				case Op:
					data_deref(key);
					state = op_eval(value.op, state);
					break;
				case List:
					data_deref(key);
					state.next = new_cons(List(state.exec), state.next);
					state.exec = value.list;
					break;
				default:
					data_deref(key);
					state.stack = new_cons(value, state.stack);
					break;
				}
				break;
			default:
				state.stack = new_cons(value,state.stack);
			}
		}
		while (!state.exec && state.next) {
			value = list_pop(&state.next);
			if (value.tag == List) {
				state.exec = value.list;
			} else {
				state.exec = new_cons(value, NULL);
			}
		}
	} while (state.exec);

	return state;
}

struct cons *generate(struct cons *dict, struct cons *exec, struct cons *stack)
{
	struct state state = {
		.dict = dict,
		.fail = new_cons(List(new_cons(List(exec), NULL)), NULL),
		.next = NULL,
		.exec = NULL,
		.stack = NULL
	};

	struct cons *retval = NULL;
	while (state.fail) {
		state.next = list_pop(&state.fail).list;
		state.exec = list_pop(&state.next).list;
		state.stack = list_ref(stack);
		state = eval(state);
		if (state.stack) {
			retval = new_cons(List(state.stack), retval);
		}
	}

	return new_cons(List(retval), stack);
}
