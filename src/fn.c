#include <el.h>

void fn_print_struct_f(FILE *f, el_fn *fn)
{
	assert(f);
	assert(fn);

	data_print_struct_f(f, fn->car);
	fprintf(f, "(");
	list_print_raw_f(f, fn->cdr);
	fprintf(f, ")");
}

void fn_built_in_print_f(FILE *f, el_built_in built_in)
{
	switch (built_in) {
	case FnUnify:
		fprintf(f, "Unify");
		break;
	case FnJoin:
		fprintf(f, "Join");
		break;
	case FnAdd:
		fprintf(f, "Add");
		break;
	default:
		fprintf(f, "fn(%u)", built_in);
		break;
	}
}

el_data fn_built_in_eval(el_dict **env_p, el_built_in built_in, el_list *args)
{
	el_data a, b;

	switch (built_in) {
	case FnUnify:
		a = list_pop(&args);
		b = list_pop(&args);
		list_deref(args);

		return data_unify(env_p, a, b);
	case FnJoin:
		a = eval(env_p, list_pop(&args));
		b = eval(env_p, list_pop(&args));
		list_deref(args);

		switch (a.type) {
		case Nil:
			data_deref(b);
			return Nil();
		case List:
			switch (b.type) {
			case Nil:
				data_deref(a);
				return Nil();
			case List:
				return List(list_join(a.list, b.list));
			default:
				return List(list_append(a.list, b));
			}
		default:
			switch (b.type) {
			case Nil:
				data_deref(a);
				return Nil();
			case List:
				return List(new_cons(a, b.list));
			default:
				return List(new_cons(a, new_cons(b, NULL)));
			}
		}
	default:
		list_deref(args);
		return Nil();
	}
}

el_data fn_eval(el_dict **env_p, el_fn *fn)
{
	if (!fn) {
		return Nil();
	}

	el_data car = fn_pop(&fn);
	switch (car.type) {
	case BuiltIn:
		return fn_built_in_eval(env_p, car.built_in, fn);
	default:
		fn_deref(fn);
		return Nil();
	}
}
