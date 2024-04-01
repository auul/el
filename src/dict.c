#include <el.h>

el_dict *new_dict_entry(el_word *key, el_data value, el_dict *left, el_dict *right)
{
	el_dict *dict = malloc(sizeof(el_dict));
	if (!dict) {
		// TODO ERROR ALLOC
		perror(strerror(errno));
		exit(1);
	}

	dict->ref = 1;
	dict->key = key;
	dict->value = value;
	dict->left = left;
	dict->right = right;
	return dict;
}

void dict_print_struct_f(FILE *f, el_dict *dict)
{
	assert(f);

	fprintf(f, "{");
	while (dict) {
		word_print_struct_f(f, dict->key);
		fprintf(f, ": ");
		data_print_struct_f(f, dict->value);

		if (dict->left) {
			fprintf(f, ", ");
			dict_print_struct_f(f, dict->left);
		}

		dict = dict->right;
		if (dict) {
			fprintf(f, ", ");
		}
	}
	fprintf(f, "}");
}

void dict_print_raw_f(FILE *f, el_dict *dict)
{
	assert(f);

	while (dict) {
		if (dict->left) {
			dict_print_raw_f(f, dict->left);
			fprintf(f, ", ");
		}

		word_print_struct_f(f, dict->key);
		fprintf(f, ": ");
		data_print_struct_f(f, dict->value);

		dict = dict->right;
		if (dict) {
			fprintf(f, ", ");
		}
	}
}

void dict_print_f(FILE *f, el_dict *dict)
{
	assert(f);

	fprintf(f, "{");
	dict_print_raw_f(f, dict);
	fprintf(f, "}");
}

void dict_print(el_dict *dict)
{
	dict_print_f(stdout, dict);
}

el_dict *dict_ref(el_dict *dict)
{
	if (dict) {
		dict->ref++;
	}
	return dict;
}

void dict_deref(el_dict *dict)
{
	while (dict) {
		dict->ref--;
		if (dict->ref) {
			return;
		}

		dict_deref(dict->left);
		word_deref(dict->key);
		data_deref(dict->value);
		el_dict *right = dict->right;
		free(dict);

		dict = right;
	}
}

el_dict *dict_anchor(el_dict *dict, el_word *key)
{
	if (!dict) {
		return NULL;
	}

	while (true) {
		int cmp = word_compare(dict->key, key);
		if (cmp < 0) {
			if (!dict->left) {
				return dict;
			}
			dict = dict->left;
		} else if (cmp > 0) {
			if (!dict->right) {
				return dict;
			}
			dict = dict->right;
		} else {
			return dict;
		}
	}
}

el_dict *dict_entry(el_dict *dict, el_word *key)
{
	dict = dict_anchor(dict, key);
	if (dict && word_equal(dict->key, key)) {
		return dict;
	}
	return NULL;
}

el_data dict_lookup(el_dict *dict, el_word *key)
{
	dict = dict_entry(dict, key);
	if (!dict) {
		return Nil();
	}
	return data_ref(dict->value);
}

el_dict *dict_edit(el_dict **dict_p, el_dict *entry)
{
	assert(dict_p);
	assert(*dict_p);
	assert(entry);

	el_dict *dict = *dict_p;
	if (dict->ref > 1) {
		dict_deref(dict);
		dict = new_dict_entry(word_ref(dict->key), data_ref(dict->value), dict_ref(dict->left), dict_ref(dict->right));
		if (*dict_p == entry) {
			*dict_p = dict;
			return dict;
		}
		*dict_p = dict;
	} else if (dict == entry) {
		return entry;
	}

	while (true) {
		int cmp = word_compare(dict->key, entry->key);
		if (cmp < 0) {
			if (dict->left->ref > 1) {
				dict_deref(dict->left);
				dict->left = new_dict_entry(word_ref(dict->left->key), data_ref(dict->left->value), dict_ref(dict->left->left), dict_ref(dict->left->right));
			}
			dict = dict->left;
		} else if (cmp > 0) {
			if (dict->right->ref > 1) {
				dict_deref(dict->right);
				dict->right = new_dict_entry(word_ref(dict->right->key), data_ref(dict->right->value), dict_ref(dict->right->left), dict_ref(dict->right->right));
			}
			dict = dict->right;
		} else {
			return dict;
		}
	}
}

el_dict *dict_define(el_dict *dict, el_word *key, el_data value)
{
	if (!dict) {
		return new_dict_entry(key, value, NULL, NULL);
	}

	el_dict *anchor = dict_anchor(dict, key);
	int cmp = word_compare(anchor->key, key);
	if (cmp < 0) {
		anchor = dict_edit(&dict, anchor);
		anchor->left = new_dict_entry(key, value, NULL, NULL);
	} else if (cmp > 0) {
		anchor = dict_edit(&dict, anchor);
		anchor->right = new_dict_entry(key, value, NULL, NULL);
	} else {
		word_deref(key);
		if (data_equal(anchor->value, value)) {
			data_deref(value);
		}

		anchor = dict_edit(&dict, anchor);
		data_deref(anchor->value);
		anchor->value = value;
	}

	return dict;
}
