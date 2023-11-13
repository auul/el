#include <el.h>

struct retval bc_match_char(struct data value, struct list *stack)
{
	if (!stack) {
		return Retval(Failure, stack);
	}

	if (stack->car.type == Char && stack->car.c == value.c) {
		return Retval(Success, stack->cdr);
	}

	return Retval(Failure, stack);
}
