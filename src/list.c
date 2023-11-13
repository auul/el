#include <el.h>

struct list *list_push(struct list *cdr, struct data car)
{
	struct list *list = alloc(List, sizeof(struct list));
	if (!list) {
		return NULL;
	}

	list->car = data_ref(car);
	list->cdr = ref(cdr);

	return list;
}

struct list *list_edit(struct list **head_p, struct list *end)
{
	struct list *prev = *head_p;
	if (!prev) {
		return NULL;
	}

	if (get_tag(prev)->ref > 0) {
		prev = list_push(prev->cdr, prev->car);
		if (!prev) {
			return NULL;
		}

		if (*head_p == end || !prev->cdr) {
			*head_p = prev;
			return prev;
		}

		*head_p = prev;
	}

	for (struct list *list = prev->cdr; list != end; list = list->cdr) {
		if (get_tag(list)->ref > 1) {
			list = list_push(list->cdr, list->car);
			if (!list) {
				return NULL;
			}

			deref(prev->cdr);
			prev->cdr = ref(list);
		}

		prev = list;
	}

	if (end) {
		if (get_tag(end)->ref > 1) {
			end = list_push(end->cdr, end->car);
			if (!end) {
				return NULL;
			}

			deref(prev->cdr);
			prev->cdr = ref(end);
		}

		prev = end;
	}

	return prev;
}
