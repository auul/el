#include <el.h>

el_word *new_word_n(const char *src, size_t bytes)
{
	assert(src);
	assert(bytes);

	el_word *word = malloc(sizeof(el_word) + bytes + 1);
	if (!word) {
		// TODO ERROR ALLOC
		perror(strerror(errno));
		exit(1);
	}

	word->ref = 1;
	memcpy(word->str, src, bytes);
	word->str[bytes] = 0;
	return word;
}

el_word *new_word(const char *src)
{
	assert(src);

	return new_word_n(src, strlen(src));
}

void word_print_struct_f(FILE *f, el_word *word)
{
	assert(f);
	assert(word);

	fprintf(f, "%s", word->str);
}

el_word *word_ref(el_word *word)
{
	assert(word);

	word->ref++;
	return word;
}

void word_deref(el_word *word)
{
	assert(word);

	word->ref--;
	if (word->ref) {
		return;
	}
	free(word);
}

int word_compare(el_word *a, el_word *b)
{
	assert(a);
	assert(b);

	if (a == b) {
		return 0;
	}
	return -strcmp(a->str, b->str);
}

bool word_equal(el_word *a, el_word *b)
{
	assert(a);
	assert(b);

	if (a == b) {
		return true;
	}
	return !strcmp(a->str, b->str);
}
