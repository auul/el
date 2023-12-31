#include <el.h>

bool check_str(const char *str)
{
	long int bracket = 0;
	for (size_t i = 0; str[i]; i++) {
		if (str[i] == '[') {
			bracket++;
		} else if (str[i] == ']') {
			bracket--;
			if (bracket < 0) {
				fprintf(stderr, "Error; unmatched parenthesis");
				return false;
			}
		}
	}
	if (bracket) {
		fprintf(stderr, "Error; unmatched parenthesis");
		return false;
	}
	return true;
}

const char *skip_whitespace(const char *str)
{
	while (*str && isspace((int)*str)) {
		str++;
	}
	return str;
}

size_t bracket_len(const char *str)
{
	if (*str != '[') {
		return 0;
	}

	long int bracket = 1;
	size_t len = 1;
	while (bracket) {
		if (str[len] == '[') {
			bracket++;
		} else if (str[len] == ']') {
			bracket--;
		}
		len++;
	}

	return len;
}

size_t token_len(const char *str)
{
	if (!*str || *str == ']' || *str == '[') {
		return 0;
	}

	size_t len = 0;
	while (str[len] && str[len] != '[' && str[len] != ']' && !isspace((int)str[len])) {
		len++;
	}

	return len;
}

bool is_int(const char *str, size_t len)
{
	if (*str == '-') {
		if (len == 1) {
			return false;
		}
		str++;
		len--;
	}

	while (len) {
		if (!isdigit((int)*str)) {
			return false;
		}
		str++;
		len--;
	}

	return true;
}

long int get_int(const char *str, size_t len)
{
	bool neg = false;
	if (*str == '-') {
		neg = true;
		str++;
		len--;
	}

	long int n = 0;
	while (len) {
		n = (10 * n) + (*str - '0');
		str++;
		len--;
	}

	return neg ? -n : n;
}

bool token_equal(const char *token, const char *str, size_t len)
{
	for (size_t i = 0; i < len; i++) {
		if (token[i] != str[i]) {
			return false;
		}
	}
	return !token[len];
}

struct cons *compile_(const char *str)
{
	struct cons *list = NULL;

	while (true) {
		str = skip_whitespace(str);
		size_t len = bracket_len(str);
		if (len) {
			list = list_append(list, List(compile_(str + 1)));
		} else {
			len = token_len(str);
			if (!len) {
				return list;
			}

			if (is_int(str, len)) {
				list = list_append(list, Int(get_int(str, len)));
			} else if (token_equal("nil", str, len)) {
				list = list_append(list, Nil());
			} else if (token_equal("nop", str, len)) {
				list = list_append(list, Op(_NOP));
			} else if (token_equal("def", str, len)) {
				list = list_append(list, Op(_DEF));
			} else if (token_equal("<<fail", str, len)) {
				list = list_append(list, Op(_GETFAIL));
			} else if (token_equal(">>fail", str, len)) {
				list = list_append(list, Op(_SETFAIL));
			} else if (token_equal("<next", str, len)) {
				list = list_append(list, Op(_POPNEXT));
			} else if (token_equal(">next", str, len)) {
				list = list_append(list, Op(_PUSHNEXT));
			} else if (token_equal("drop", str, len)) {
				list = list_append(list, Op(_DROP));
			} else if (token_equal("dup", str, len)) {
				list = list_append(list, Op(_DUP));
			} else if (token_equal("swap", str, len)) {
				list = list_append(list, Op(_SWAP));
			} else if (token_equal("i", str, len)) {
				list = list_append(list, Op(_I));
			} else if (token_equal("fork", str, len)) {
				list = list_append(list, Op(_FORK));
			} else if (token_equal("test", str, len)) {
				list = list_append(list, Op(_TEST));
			} else if (token_equal("loop", str, len)) {
				list = list_append(list, Op(_LOOP));
			} else if (token_equal("gen", str, len)) {
				list = list_append(list, Op(_GEN));
			} else if (token_equal("invert", str, len)) {
				list = list_append(list, Op(_INVERT));
			} else if (token_equal("contain", str, len)) {
				list = list_append(list, Op(_CONTAIN));
			} else if (token_equal("unit", str, len)) {
				list = list_append(list, Op(_UNIT));
			} else if (token_equal("pop", str, len)) {
				list = list_append(list, Op(_POP));
			} else if (token_equal("push", str, len)) {
				list = list_append(list, Op(_PUSH));
			} else if (token_equal("cat", str, len)) {
				list = list_append(list, Op(_CAT));
			} else if (token_equal("choose", str, len)) {
				list = list_append(list, Op(_CHOOSE));
			} else if (token_equal("=", str, len)) {
				list = list_append(list, Op(_EQUAL));
			} else if (token_equal("+", str, len)) {
				list = list_append(list, Op(_ADD));
			} else if (token_equal("-", str, len)) {
				list = list_append(list, Op(_SUBTRACT));
			} else if (token_equal("*", str, len)) {
				list = list_append(list, Op(_MULTIPLY));
			} else if (token_equal("/", str, len)) {
				list = list_append(list, Op(_DIVIDE));
			} else {
				list = list_append(list, Token(new_str_n(str, len)));
			}
		}
		str += len;
	}
}

struct cons *compile(const char *str)
{
	if (!check_str(str)) {
		return NULL;
	}
	return compile_(str);
}

int main(int argc, char **args)
{
	if (argc < 2) {
		return 0;
	}

	struct state state = {0};
	state.exec = compile(args[1]);
	state_print(state);
	state_print(eval(state));

	return 0;
}
