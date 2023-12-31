#include <el.h>

struct cons *new_str_n(const char *src, size_t len)
{
	struct cons *dest = NULL;
	for (size_t i = 1; i <= len; i++) {
		dest = new_cons(Int(src[len - i]), dest);
	}
	return dest;
}

struct cons *new_str(const char *src)
{
	return new_str_n(src, strlen(src));
}

void token_print(struct cons *str)
{
	while (str) {
		printf("%c", (char)str->car.i_num);
		str = str->cdr;
	}
}
