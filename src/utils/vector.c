#include "vector.h"

void free_data(void* data){
    free(data);
}

Vector* create_vector(int initCapacity, size_t element_size){
    Vector* vector = malloc(sizeof(Vector));
    vector->element_size = element_size;
    vector->capacity = initCapacity;
    vector->size = 0;
    vector->data = malloc(initCapacity * element_size);
    vector->free_data = free_data;
    return vector;
}

Vector* create_vector_with_free(int initCapacity, size_t element_size, void (*free_data)(void*)){
    Vector* vector = malloc(sizeof(Vector));
    vector->element_size = element_size;
    vector->capacity = initCapacity;
    vector->size = 0;
    vector->data = malloc(initCapacity * element_size);
    vector->free_data = free_data;
    return vector;
}

void vector_push(Vector* vector, void* element){
    if(vector->size == vector->capacity){
        vector->capacity *= 2;
        vector->data = realloc(vector->data, vector->capacity * vector->element_size);
    }
    memcpy(vector->data + vector->size * vector->element_size, element, vector->element_size);
    vector->size += 1;
}

void* vector_get(Vector* vector, size_t index){
    if(index >= vector->size){
        return NULL;
    }
    return vector->data + index * vector->element_size;
}

void vector_set(Vector* vector, size_t index, void* element){
    if(index >= vector->size){
        return;
    }
    memcpy(vector->data + index * vector->element_size, element, vector->element_size);
}

void vector_delete(Vector* vector, size_t index){
    if(index >= vector->size){
        return;
    }
    vector->free_data(vector->data + index * vector->element_size);
    memcpy(vector->data + index * vector->element_size, vector->data + (index + 1) * vector->element_size, (vector->size - index - 1) * vector->element_size);
    vector->size -= 1;
}

void vector_clear(Vector* vector){
    if(vector->data != NULL) {
        for (int i = 0; i < vector->size; i++) {
            vector->free_data(vector->data + i * vector->element_size);
        }
        vector->size = 0;
    }
}

void vector_free(Vector* vector){
    vector_clear(vector);
    free(vector);
}

void vector_pop(Vector* vector){
    if(vector->size == 0){
        return;
    }
    vector->free_data(vector->data + (vector->size - 1) * vector->element_size);
    vector->size -= 1;
}

VectorIterator* create_vector_iterator(Vector* vector){
    if(vector == NULL){
        return NULL;
    }
    VectorIterator* iterator = malloc(sizeof(VectorIterator));
    iterator->vector = vector;
    iterator->index = 0;
    return iterator;
}

void free_vector_iterator(VectorIterator* iterator){
    free(iterator);
}

bool vector_iterator_has_next(VectorIterator* iterator){
    return iterator->index < iterator->vector->size;
}

void* vector_iterator_next_data(VectorIterator* iterator){
    if(vector_iterator_has_next(iterator)){
        void* data = iterator->vector->data + iterator->index * iterator->vector->element_size;
        iterator->index += 1;
        return data;
    }
    return NULL;
}