#include <el.h>

void int_print_struct_f(FILE *f, el_int i[2])
{
	assert(f);

	if (i[0] == i[1]) {
		fprintf(f, "%i", i[0]);
		return;
	}
	fprintf(f, "%i..%i", i[0], i[1]);
}

int int_compare(el_int a[2], el_int b[2])
{
	if (a[0] > b[0]) {
		return -1;
	} else if (a[0] < b[0]) {
		return 1;
	} else if (a[1] > b[1]) {
		return -1;
	} else if (a[1] < b[1]) {
		return 1;
	}
	return 0;
}

bool int_equal(el_int a[2], el_int b[2])
{
	return a[0] == b[0] && a[1] == b[1];
}


el_data int_merge(el_int a[2], el_int b[2])
{
	if (a[1] < b[0] - 1) {
		return Nil();
	} else if (a[0] > b[1] + 1) {
		return Nil();
	} else if (a[0] < b[0]) {
		if (a[1] > b[1]) {
			return IntRange(a[0], a[1]);
		}
		return IntRange(a[0], b[1]);
	} else if (a[1] > b[1]) {
		return IntRange(b[0], a[1]);
	}
	return IntRange(b[0], b[1]);
}

el_data int_unify_int(el_int a[2], el_int b[2])
{
	if (b[0] > a[0]) {
		a[0] = b[0];
	}

	if (b[1] < a[1]) {
		a[1] = b[1];
	}

	if (a[0] > a[1]) {
		return Nil();
	}

	return IntRange(a[0], a[1]);
}

el_data int_choose(el_data *data_p)
{
	el_data data = *data_p;
	if (data.i[0] == data.i[1]) {
		*data_p = Nil();
		return data;
	}

	data_p->i[0] += 1;
	data.i[1] = data.i[0];
	return data;
}
