#include <el.h>

struct tag *gc_garbage;

void *alloc(enum type type, size_t size)
{
	struct tag *tag = malloc(sizeof(struct tag) + size);
	if (!tag) {
		perror(strerror(errno));
		return NULL;
	}

	tag->prev = gc_garbage;
	gc_garbage = tag;

	tag->ref = 0;
	tag->size = size;
	tag->type = type;

	return tag->ptr;
}

void *ref(void *ptr)
{
	if (ptr) {
		get_tag(ptr)->ref++;
	}
	return ptr;
}

void *deref(void *ptr)
{
	if (ptr && get_tag(ptr)->ref) {
		get_tag(ptr)->ref--;
	}
	return ptr;
}

void dump_garbage(struct tag *end_at)
{
	size_t total = 0;

	printf("Dumping Garbage...\n");
	for (struct tag *tag = gc_garbage; tag != end_at; tag = tag->prev) {
		struct data value = {
			.type = tag->type,
			.ptr = tag->ptr
		};
		printf("%lu:", tag->ref);
		data_echo(value);
		printf("\n");

		total += sizeof(struct tag) + tag->size;
	}
	printf("Total: %lu bytes\n", total);
}
