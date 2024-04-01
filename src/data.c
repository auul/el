#include <el.h>

void data_print_struct_f(FILE *f, el_data data)
{
	assert(f);

	switch (data.type) {
	case Nil:
		fprintf(f, "nil");
		break;
	case Any:
		switch (data.any) {
		case Nil:
			fprintf(f, "Any");
			break;
		case Char:
			fprintf(f, "Char");
			break;
		case List:
			fprintf(f, "List");
			break;
		default:
			fprintf(f, "Any(%u)", data.any);
			break;
		}
		break;
	case Char:
		char_print_struct_f(f, data.c);
		break;
	case Int:
		int_print_struct_f(f, data.i);
		break;
	case Word:
		word_print_struct_f(f, data.word);
		break;
	case List:
		list_print_struct_f(f, data.list);
		break;
	case Choice:
		choice_print_struct_f(f, data.choice);
		break;
	case Fn:
		fn_print_struct_f(f, data.fn);
		break;
	case BuiltIn:
		fn_built_in_print_f(f, data.built_in);
		break;
	default:
		fprintf(f, "%p", data.ptr);
		break;
	}
}

void data_print_struct(el_data data)
{
	data_print_struct_f(stdout, data);
}

void data_print(el_data data)
{
	data_print_struct(data);
}

el_data data_ref(el_data data)
{
	switch (data.type) {
	case Word:
		word_ref(data.word);
		break;
	case List:
		list_ref(data.list);
		break;
	case Choice:
		choice_ref(data.choice);
		break;
	case Fn:
		fn_ref(data.fn);
		break;
	default:
		break;
	}

	return data;
}

void data_deref(el_data data)
{
	switch (data.type) {
	case Word:
		word_deref(data.word);
		break;
	case List:
		list_deref(data.list);
		break;
	case Choice:
		choice_deref(data.choice);
		break;
	case Fn:
		fn_deref(data.fn);
		break;
	default:
		break;
	}
}

int data_compare(el_data a, el_data b)
{
	if (a.type > b.type) {
		return -1;
	} else if (a.type < b.type) {
		return 1;
	}

	switch (a.type) {
	case Nil:
		return 0;
	case Any:
		if (a.any > b.any) {
			return -1;
		} else if (a.any < b.any) {
			return 1;
		}
		return 0;
	case Char:
		return char_compare(a.c, b.c);
	case Int:
		return int_compare(a.i, b.i);
	case Word:
		return word_compare(a.word, b.word);
	case List:
		return list_compare(a.list, b.list);
	case Choice:
		return choice_compare(a.choice, b.choice);
	case Fn:
		return fn_compare(a.fn, b.fn);
	default:
		if (a.ptr > b.ptr) {
			return -1;
		} else if (a.ptr < b.ptr) {
			return 1;
		}
		return 0;
	}
}

bool data_equal(el_data a, el_data b)
{
	if (a.type != b.type) {
		return false;
	}

	switch (a.type) {
	case Nil:
		return true;
	case Any:
		return a.any == b.any;
	case Char:
		return char_equal(a.c, b.c);
	case Int:
		return int_equal(a.i, b.i);
	case Word:
		return word_equal(a.word, b.word);
	case List:
		return list_equal(a.list, b.list);
	case Choice:
		return choice_equal(a.choice, b.choice);
	case Fn:
		return fn_equal(a.fn, b.fn);
	default:
		return a.ptr == b.ptr;
	}
}

el_data data_merge(el_data a, el_data b)
{
	switch (b.type) {
	case Nil:
		return a;
	case Any:
		if (b.any == Nil || b.any == a.type) {
			return b;
		}
		return Nil();
	case Choice:
		if (a.type == Choice) {
			return choice_simplify(choice_join(choice_ref(a.choice), choice_ref(b.choice)));
		}
		return choice_simplify(new_cons(data_ref(a), choice_ref(b.choice)));
	default:
		break;
	}

	switch (a.type) {
	case Any:
		if (a.any == Nil || a.any == b.type) {
			return a;
		}
		return Nil();
	case Char:
		if (b.type != Char) {
			return Nil();
		}
		return char_merge(a.c, b.c);
	case Int:
		if (b.type != Int) {
			return Nil();
		}
		return int_merge(a.i, b.i);
	case Choice:
		return choice_simplify(new_cons(data_ref(b), choice_ref(a.choice)));
	default:
		return Nil();
	}
}

el_data data_unify(el_dict **env_p, el_data a, el_data b)
{
	if (data_equal(a, b)) {
		data_deref(b);
		return a;
	}

	if (a.type == Word) {
		if (b.type == Word) {
			data_deref(a);
			data_deref(b);
			return Nil();
		}
		*env_p = dict_define(*env_p, a.word, data_ref(b));
		return b;
	} else if (b.type == Word) {
		*env_p = dict_define(*env_p, b.word, data_ref(a));
		return a;
	}

	switch (b.type) {
	case Nil:
		data_deref(a);
		return Nil();
	case Any:
		if (b.any == Nil || b.any == a.type) {
			return a;
		}
		break;
	case Choice:
		return choice_unify(env_p, b.choice, a);
	default:
		break;
	}

	switch (a.type) {
	case Any:
		if (a.any == Nil || a.any == b.type) {
			return b;
		}
		break;
	case Char:
		if (b.type == Char) {
			return char_unify_char(a.c, b.c);
		}
		break;
	case Int:
		if (b.type == Int) {
			return int_unify_int(a.i, b.i);
		}
		break;
	case List:
		if (b.type == List) {
			return list_unify_list(env_p, a.list, b.list);
		}
		break;
	case Choice:
		return choice_unify(env_p, a.choice, b);
	default:
		break;
	}

	data_deref(a);
	data_deref(b);
	return Nil();
}

el_data data_choose(el_data *data_p)
{
	el_data data;
	switch (data_p->type) {
	case Char:
		data = char_choose(data_p);
		break;
	case Int:
		data = int_choose(data_p);
		break;
	case List:
		data = list_choose(data_p);
		break;
	case Choice:
		data = choice_choose(data_p);
		break;
	default:
		data = *data_p;
		*data_p = Nil();
		break;
	}

	while (data_p->type != Nil) {
		el_data next = data_choose(&data);
		if (data.type == Nil) {
			return next;
		}
		*data_p = choice_rejoin(data, *data_p);
		data = next;
	}

	return data;
}

el_data eval(el_dict **env_p, el_data ctrl) 
{
	switch (ctrl.type) {
	case Word:
		return eval(env_p, dict_lookup(*env_p, ctrl.word));
	case List:
		return list_eval(env_p, ctrl.list);
	case Choice:
		return choice_eval(env_p, ctrl.choice);
	case Fn:
		return fn_eval(env_p, ctrl.fn);
	default:
		return ctrl;
	}
}
