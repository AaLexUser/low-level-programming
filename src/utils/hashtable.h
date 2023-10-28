#pragma once
#include <limits.h>
#include <stddef.h>
#include <stdint.h>

#define HT(key_type, value_type) struct { \
	size_t size, used, max_used, capacity; \
	char *flags; \
	key_type *keys; \
	value_type *values; \
}

// flag = 1 - occupied flag = 2 - deleted flag = 3 - free

#define ht_init(h) do { \
    (h).size = (h).used = (h).max_used = (h).capacity = 0; \
    (h).flags = NULL;   \
    (h).keys = NULL;    \
    (h).values = NULL;\
} while (0)

#define ht_destroy(h) do{ \
    free((h).values);     \
    free((h).keys);       \
    free((h).flags);      \
} while (0)

#define ht_size(h) ((h).size)
#define ht_max_used(h) ((h).max_used)
#define ht_used(h) ((h).used)
#define ht_capacity(h) ((h).capacity)
#define ht_key(h, index) ((h).keys[(index)])
#define ht_value(h, index) ((h).values[(index)]

#define linear_probe(index, step) ((index) + (step))
#define quadratic_probe(index, step) ((index) + (step) * (step))

#ifndef HT_STEP_INCREMENT
#define HT_STEP_INCREMENT 2
#endif
#if HT_STEP_INCREMENT == 1
#define ht_probe(index, step) linear_probe((index), (step))
#else 
#define ht_probe(index, step) quadratic_probe((index), (step))
#endif

#define ht_clear(h) do { \
    (h).size = 0; \
    (h).used = 0; \
    if ((h).flags) { memset((h).flags, 0, (h).capacity); } \
} while (0)

#define ht_reserve(h, key_type, value_type, new_capacity, success, hash_func) do {\
    if((new_capacity) <= (h).max_used) {\
        (success) = true; \
        break;\
    }\
    size_t ht_new_capacity = (new_capacity);\
    roundupsize(ht_new_capacity);\
    if (ht_new_capacity < (new_capacity))  { /*Integer overflow*/ \
        (success) = false;\
        break;\
    }\
    size_t ht_new_max_used = (ht_new_capacity >> 1) + (ht_new_capacity >> 2); /*3/4 of the new capacity*/ \
    if (ht_new_max_used < (new_capacity)) {\
        ht_new_capacity <<= 1;\
        if (ht_new_capacity < (new_capacity))  { /*Integer overflow*/ \
            (success) = false;\
            break;\
        }\
        ht_new_max_used = (ht_new_capacity >> 1) + (ht_new_capacity >> 2); \
    }\
    char *ht_new_flags = malloc(ht_new_capacity); \
    if (!ht_new_flags){\
        (success) = false; \
        break;\
    }\
    key_type *ht_new_keys = malloc(ht_new_capacity * sizeof(key_type)); \
    if (!ht_new_keys) {\
        free(ht_new_flags);\
        (success) = false; \
        break;\
    }\
    value_type *ht_new_values = malloc(ht_new_capacity * sizeof(value_type));\
    if(!ht_new_values) {\
        free(ht_new_flags);\
        free(ht_new_keys);\
        (success) = false; \
        break;\
    }\
    memset(ht_new_flags, 0, ht_new_capacity);\
    size_t ht_mask = ht_new_capacity - 1;\
    for(size_t ht_i = 0; ht_i < (h).capacity; ht_i++){\
        if((h).flags[ht_i] != 1) continue;\
        size_t ht_index = hash_func((h).keys[ht_i]) & ht_mask;\
        size_t ht_step = 0;\
        while(ht_new_flags[ht_index]){\
            ht_index = ht_probe(ht_index, ht_step) & ht_mask;\
        }\
        ht_new_flags[ht_index] = 1;\
        ht_new_keys[ht_index] = (h).keys[ht_i];\
        ht_new_values[ht_index] = (h).values[ht_i];\
    }\
    free((h).flags);\
    free((h).keys);\
    free((h).values);\
    (h).flags = ht_new_flags;\
    (h).keys = ht_new_keys;\
    (h).values = ht_new_values;\
    (h).capacity = ht_new_capacity;\
    (h).max_used = ht_new_max_used;\
    (h).used = (h).size;\
    (success) = true;\
} while(0)

#define ht_get(h, key, ht_result, hash_func, eq_func) do { \
      if(!(h).size){                                    \
        (ht_result) = 0;                                   \
        break;\
      }                                                 \
      size_t ht_mask = (h).capacity - 1;                \
      (ht_result) = hash_func(key) & ht_mask;               \
      size_t ht_step = 0;\
      while((h).flags[(ht_result)] == 2 || ((h).flags[(ht_result)] == 1 && !eq_func(h)(keys[(ht_result)], (key))))\
        {\
            (ht_result) = ht_probe(ht_result, ht_step) & ht_mask;\
        }\
}\

#define ht_valid(h, index) ((h).flags && (h).flags[(index)] == 1)

/**
 * Insert an element inside the hash table and return its index.
 * 
 * absent = 1 - key was absent and was added
 * absent = 0 - key was present
 * absent = -1 - error
*/
#define ht_put(h, key_type, value_type, key, index, absent, hash_func, eq_func) do {\
    bool ht_success;\
    size_t ht_new_size = (h).size ? (h).size + 1 : 2;\
    ht_reserve((h), key_type, value_type, ht_new_size, ht_success, hash_func);\
    if (!ht_success) {\
        (absent) = -1; \
        break;\
    }\
    size_t ht_mask = (h).capacity - 1;\
    (index) = hash_func(key) & ht_mask;\
    size_t ht_step = 0; \
    while((h).flags[(index)] == 1 || !eq_func(h)(keys[(result)], (key)))){\
        (index) = ((index) + (++ht_step)) & ht_mask;\
    }\
    if((h).flags[(index)] == 1){\
        (absent) = 0;\
    } else {\
        (h).flags[(index)] = 1; \
        (h).keys[(index)] = (key); \
        (h).size++;\
        (absent) = 1;\
    }\
} while(0)

#define ht_remove(h, index) do {\
    (h).flags[(index)] = 2;\
    (h).size--;\
} while(0)

#define ht_begin(h) (0)
#define ht_end(h) ((h).capacity)
#define ht_key_at(h, index) ((h).keys[(index)])
#define ht_value_at(h, index) ((h).values[(index)])


#define roundup32(x) (--(x), (x)|=(x)>>1, (x)|=(x)>>2, (x)|=(x)>>4, (x)|=(x)>>8, (x)|=(x)>>16, ++(x))
#define roundup64(x) (--(x), (x)|=(x)>>1, (x)|=(x)>>2, (x)|=(x)>>4, (x)|=(x)>>8, (x)|=(x)>>16, (x)|=(x)>>32, ++(x))

#if SIZE_MAX == UINT64_MAX
#define roundupsize(x) roundup64(x)
#elif SIZE_MAX == UINT32_MAX
#define roundupsize(x) roundup32(x)
#endif

#define ht_int_hash(x) ((size_t) (x))
#define ht_int_eq(x, y) ((x) == (y))

static inline size_t ht_str_hash(const char *str) {
    size_t hash = (size_t) *str;
	if (hash) {
		for(++str; *str; ++str) {
			hash = (hash << 5) - hash + (size_t) *str;
		}
	}
	return hash;
}

#define ht_str_eq(a, b) (strcmp((a), (b)) == 0)

static inline size_t ht_nearest_valid_index(const char *flags, size_t capacity, size_t index) {
	while (index < capacity && flags[index] != 1) {
		index++;
	}
	return index;
}

#define ht_for_each(h, index) for ( \
	size_t index = ht_nearest_valid_index((h).flags, (h).capacity, ht_begin((h))); \
	(index) != ht_end((h)) && ht_valid((h), index); \
	(index)++, (index) = ht_nearest_valid_index((h).flags, (h).capacity, (index)) \
)

#define ht_reserve_int(h, value_type, new_capacity, success) \
ht_reserve((h), int, value_type, (new_capacity), (success), ht_int_hash, ht_int_eq)

#define ht_get_int(h, key, result) \
ht_get((h), (key), (result), ht_int_hash, ht_int_eq)

#define ht_put_int(h, value_type, key, index, absent) \
ht_put((h), int, value_type, (key), (index), (absent), ht_int_hash, ht_int_eq)






