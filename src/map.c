#include <el.h>

struct cons *new_map(struct data key, struct data value)
{
	return new_cons(value, new_cons(key, NULL));
}

struct data map_key(struct cons *map)
{
	if (map && map->car.tag == Map) {
		return map->car.map->cdr->car;
	}
	return Nil();
}

struct data map_value(struct cons *map)
{
	if (map && map->car.tag == Map) {
		return map->car.map->car;
	}
	return Nil();
}

void map_print(struct cons *map)
{
	data_print(map_key(map));
	printf(":");
	data_print(map_value(map));
}

struct cons *map_define(struct cons *map, struct data key, struct data value)
{
	return new_cons(Map(new_map(key, value)), map);
}

struct cons *map_find(struct cons *map, struct data key)
{
	while (map) {
		if (map->car.tag == Map && data_equal(map_key(map), key)) {
			return map;
		}
		map = map->cdr;
	}
	return NULL;
}

struct data map_lookup(struct cons *map, struct data key)
{
	return map_value(map_find(map, key));
}
