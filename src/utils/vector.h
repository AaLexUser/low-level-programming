#ifndef VECTOR_H
#define VECTOR_H
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
typedef struct{
    void* data;
    int size;
    int capacity;
    int element_size;
    void (*free_data)(void*);
} Vector;

typedef struct{
    Vector* vector;
    int index;
} VectorIterator;

Vector* create_vector(int initCapacity, size_t element_size);
Vector* create_vector_with_free(int initCapacity, size_t element_size, void (*free_data)(void*));
void vector_push(Vector* vector, void* element);
void* vector_get(Vector* vector, size_t index);
void vector_set(Vector* vector, size_t index, void* element);
void vector_delete(Vector* vector, size_t index);
void vector_free(Vector* vector);
VectorIterator* create_vector_iterator(Vector* vector);
void vector_pop(Vector* vector);
void free_vector_iterator(VectorIterator* iterator);
bool vector_iterator_has_next(VectorIterator* iterator);
void* vector_iterator_next_data(VectorIterator* iterator);

#endif