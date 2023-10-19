#pragma once

#define HT(key_type, value_type){ \
    size_t size, max_size, capacity; \
    char *flags;                    \
    key_type *keys;               \
    value_type *values;           \
}

#define ht_init(h) do { \
    (h).size = (h).max_size = (h).capacity = 0; \
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
#define ht_max_size(h) ((h).max_size)
#define ht_capacity(h) ((h).capacity)
#define ht_key(h, index) ((h).keys[(index)])
#define ht_value(h, index) ((h).values[(index)]

#define ht_clear(h) do { \
    (h).size = 0; \
    if ((h).flags) { memset((h).flags, 0, (h).capacity); } \
} while (0)

#define ht_get(h, key, result, hash_func, eq_func) do { \
      if(!(h).size){                                    \
        (result) = 0;                                   \
        break;\
      }                                                 \
      size_t ht_mask = (h).capacity - 1;                \
      (result) = \
}

