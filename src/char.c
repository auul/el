#include <el.h>

#define UTF8_XBYTE 128
#define UTF8_XMASK 192
#define UTF8_2BYTE 192
#define UTF8_2MASK 224
#define UTF8_3BYTE 224
#define UTF8_3MASK 240
#define UTF8_4BYTE 240
#define UTF8_4MASK 248

#define UTF8_SHIFT 6

#define UTF8_4MAX 2097151
#define UTF8_3MAX   65535
#define UTF8_2MAX    2047
#define UTF8_1MAX     127

unsigned utf8_char_size(const char *src)
{
	assert(src);

	if ((src[0] & UTF8_4MASK) == UTF8_4BYTE) {
		if (((src[1] & UTF8_XMASK) == UTF8_XBYTE) &&
		    ((src[2] & UTF8_XMASK) == UTF8_XBYTE) &&
		    ((src[3] & UTF8_XMASK) == UTF8_XBYTE)) {
			return 4;
		}
	} else if ((src[0] & UTF8_3MASK) == UTF8_3BYTE) {
		if (((src[1] & UTF8_XMASK) == UTF8_XBYTE) &&
		    ((src[2] & UTF8_XMASK) == UTF8_XBYTE)) {
			return 3;
		}
	} else if ((src[0] & UTF8_2MASK) == UTF8_2BYTE) {
		if ((src[1] & UTF8_XMASK) == UTF8_XBYTE) {
			return 2;
		}
	} else if (!(src[0] & UTF8_XBYTE)) {
		return 1;
	}

	// TODO ERROR ILLEGAL UTF8 SEQUENCE
	return 0;
}

size_t utf8_len(const char *src)
{
	if (!src) {
		return 0;
	}

	size_t len = 0;
	while (*src) {
		unsigned char_size = utf8_char_size(src);
		if (!char_size) {
			return 0;
		}

		src += char_size;
		len++;
	}
	return len;
}

size_t utf8_size(const char *src, size_t len)
{
	if (!len) {
		return 0;
	}

	assert(src);
	size_t size = 0;
	while (len) {
		assert(src[size]);

		size += utf8_char_size(src + size);
		len--;
	}
	return size;
}

el_char char_read(const char **src_p)
{
	assert(src_p);
	assert(*src_p);

	const char *src = *src_p;
	switch (utf8_char_size(src)) {
	case 0:
		return 0;
	case 1:
		*src_p += 1;
		return (el_char)src[0];
	case 2:
		*src_p += 2;
		return ((el_char)((uint8_t)src[0] & ~UTF8_2MASK) << (1 * UTF8_SHIFT)) |
		       ((el_char)((uint8_t)src[1] & ~UTF8_XMASK) << (0 * UTF8_SHIFT));
	case 3:
		*src_p += 3;
		return ((el_char)((uint8_t)src[0] & ~UTF8_3MASK) << (2 * UTF8_SHIFT)) |
		       ((el_char)((uint8_t)src[1] & ~UTF8_XMASK) << (1 * UTF8_SHIFT)) |
		       ((el_char)((uint8_t)src[2] & ~UTF8_XMASK) << (0 * UTF8_SHIFT));
	case 4:
		*src_p += 4;
		return ((el_char)((uint8_t)src[0] & ~UTF8_4MASK) << (3 * UTF8_SHIFT)) |
		       ((el_char)((uint8_t)src[1] & ~UTF8_XMASK) << (2 * UTF8_SHIFT)) |
		       ((el_char)((uint8_t)src[2] & ~UTF8_XMASK) << (1 * UTF8_SHIFT)) |
		       ((el_char)((uint8_t)src[3] & ~UTF8_XMASK) << (0 * UTF8_SHIFT));
	default:
		return 0;
	}
}

char *char_write(char *dest, el_char c)
{
	assert(dest);
	assert(c);

	if (c > UTF8_4MAX) {
		assert(c <= UTF8_4MAX);
		// TODO ERROR CHAR OUT OF BOUNDS
		return NULL;
	} else if (c > UTF8_3MAX) {
		dest[0] = UTF8_4BYTE | (uint8_t)(c >> (3 * UTF8_SHIFT));
		dest[1] = UTF8_XBYTE | (uint8_t)((c >> (2 * UTF8_SHIFT)) & ~UTF8_XMASK);
		dest[2] = UTF8_XBYTE | (uint8_t)((c >> (1 * UTF8_SHIFT)) & ~UTF8_XMASK);
		dest[3] = UTF8_XBYTE | (uint8_t)((c >> (0 * UTF8_SHIFT)) & ~UTF8_XMASK);
		return dest + 4;
	} else if (c > UTF8_2MAX) {
		dest[0] = UTF8_3BYTE | (uint8_t)(c >> (2 * UTF8_SHIFT));
		dest[1] = UTF8_XBYTE | (uint8_t)((c >> (1 * UTF8_SHIFT)) & ~UTF8_XMASK);
		dest[2] = UTF8_XBYTE | (uint8_t)((c >> (0 * UTF8_SHIFT)) & ~UTF8_XMASK);
		return dest + 3;
	} else if (c > UTF8_1MAX) {
		dest[0] = UTF8_2BYTE | (uint8_t)(c >> (1 * UTF8_SHIFT));
		dest[1] = UTF8_XBYTE | (uint8_t)((c >> (0 * UTF8_SHIFT)) & ~UTF8_XMASK);
		return dest + 2;
	} else if (c > 0) {
		dest[0] = (uint8_t)c;
		return dest + 1;
	}

	// TODO ERROR NULL CHAR
	return NULL;
}

void char_print_raw_f(FILE *f, el_char c)
{
	assert(f);

	char buf[5] = {0};
	char_write(buf, c);
	fprintf(f, "%s", buf);
}

void char_print_esc_f(FILE *f, el_char c)
{
	assert(f);

	switch (c) {
	case '\a':
		fprintf(f, "\\a");
		break;
	case '\b':
		fprintf(f, "\\b");
		break;
	case '\e':
		fprintf(f, "\\e");
		break;
	case '\f':
		fprintf(f, "\\f");
		break;
	case '\n':
		fprintf(f, "\\n");
		break;
	case '\r':
		fprintf(f, "\\r");
		break;
	case '\t':
		fprintf(f, "\\t");
		break;
	case '\v':
		fprintf(f, "\\v");
		break;
	case '\\':
		fprintf(f, "\\\\");
		break;
	case ' ':
		fprintf(f, " ");
		break;
	default:
		if (c > UTF8_1MAX || isgraph((int)c)) {
			char_print_raw_f(f, c);
		} else {
			fprintf(f, "\\%#.2x", c);
		}
		break;
	}
}

void char_print_struct_f(FILE *f, el_char c[2])
{
	assert(f);

	fprintf(f, "\"");

	if (c[0] == '"') {
		fprintf(f, "\\\"");
	} else {
		char_print_esc_f(f, c[0]);
	}

	if (c[0] != c[1]) {
		fprintf(f, "\"..\"");
		if (c[1] == '"') {
			fprintf(f, "\\\"");
		} else {
			char_print_esc_f(f, c[1]);
		}
	}

	fprintf(f, "\"");
}

int char_compare(el_char a[2], el_char b[2])
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

bool char_equal(el_char a[2], el_char b[2])
{
	return a[0] == b[0] && a[1] == b[1];
}

el_data char_merge(el_char a[2], el_char b[2])
{
	if (a[1] < b[0] - 1) {
		return Nil();
	} else if (a[0] > b[1] + 1) {
		return Nil();
	} else if (a[0] < b[0]) {
		if (a[1] > b[1]) {
			return CharRange(a[0], a[1]);
		}
		return CharRange(a[0], b[1]);
	} else if (a[1] > b[1]) {
		return CharRange(b[0], a[1]);
	}
	return CharRange(b[0], b[1]);
}

el_data char_unify_char(el_char a[2], el_char b[2])
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

	return CharRange(a[0], a[1]);
}

el_data char_choose(el_data *data_p)
{
	el_data data = *data_p;
	if (data.c[0] == data.c[1]) {
		*data_p = Nil();
		return data;
	}

	data_p->c[0] += 1;
	data.c[1] = data.c[0];
	return data;
}
