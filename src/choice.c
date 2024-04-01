#include <el.h>

void choice_print_struct_f(FILE *f, el_choice *choice)
{
	fprintf(f, "(");
	while (choice) {
		data_print(choice->car);
		choice = choice->cdr;
		if (choice) {
			fprintf(f, " | ");
		}
	}
	fprintf(f, ")");
}

el_data choice_simplify(el_choice *choice)
{
	if (!choice) {
		return Nil();
	} else if (!choice->cdr) {
		return choice_pop(&choice);
	}

	for (el_choice *a = choice; a->cdr; a = a->cdr) {
		for (el_choice *b = a->cdr; b; b = b->cdr) {
			el_data merge = data_merge(a->car, b->car);
			if (merge.type == Choice) {
				choice = choice_delete(choice, a);
				choice = choice_delete(choice, b);
				return choice_simplify(choice_join(merge.choice, choice));
			} else if (merge.type != Nil) {
				choice = choice_delete(choice, a);
				choice = choice_delete(choice, b);
				return choice_simplify(new_cons(merge, choice));
			}
			printf("\n");
		}
	}

	if (choice) {
		if (choice->cdr) {
			return Choice(choice);
		}
		return choice_pop(&choice);
	}
	return Nil();
}

el_data choice_unify(el_dict **env_p, el_choice *a, el_data b)
{
	el_choice *out = NULL;

	while (a) {
		el_data car = choice_pop(&a);
		el_data value = data_unify(env_p, car, data_ref(b));
		switch (value.type) {
		case Nil:
			break;
		case Choice:
			out = choice_join(value.choice, out);
			break;
		default:
			out = new_cons(value, out);
			break;
		}
	}
	data_deref(b);

	return choice_simplify(out);
}

el_data choice_choose(el_data *data_p)
{
	assert(data_p);

	el_data car = choice_pop(&data_p->choice);
	if (data_p->choice) {
		if (!data_p->choice->cdr) {
			*data_p = choice_pop(&data_p->choice);
		}
	} else {
		*data_p = Nil();
	}
	return car;
}

el_data choice_rejoin(el_data ctrl, el_data alt)
{
	el_data merge = data_merge(ctrl, alt);
	if (merge.type != Nil) {
		data_deref(ctrl);
		data_deref(alt);
		return merge;
	}

	switch (ctrl.type) {
	case Nil:
		return alt;
	case Choice:
		switch (alt.type) {
		case Nil:
			return ctrl;
		case Choice:
			return choice_simplify(choice_join(ctrl.choice, alt.choice));
		default:
			return choice_simplify(new_cons(alt, ctrl.choice));
		}
	default:
		switch (alt.type) {
		case Nil:
			return ctrl;
		case Choice:
			return choice_simplify(new_cons(ctrl, alt.choice));
		default:
			return choice_simplify(new_cons(ctrl, new_cons(alt, NULL)));
		}
	}
}

el_data choice_eval(el_dict **env_p, el_choice *choice)
{
	if (!choice) {
		return Nil();
	} else if (!choice->cdr) {
		return eval(env_p, choice_pop(&choice));
	}

	el_data alt = Choice(choice);
	el_data ctrl = eval(env_p, choice_choose(&alt));

	if (ctrl.type == Nil) {
		return eval(env_p, alt);
	}
	return choice_rejoin(ctrl, alt);
}
