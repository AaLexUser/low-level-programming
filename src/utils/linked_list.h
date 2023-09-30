#pragma once
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
typedef struct Node{
    void* data;
    struct Node* next;
} Node;

typedef struct{
    struct Node* head;
    struct Node* tail;
    size_t size;
    void (*free_data)(void*);
} LinkedList;

typedef struct{
    LinkedList* list;
    Node* prev;
    Node* current;
    Node* next;
} LinkedListIterator;

LinkedList* create_linked_list();
LinkedList* create_linked_list_with_free(void (*free)(void*));
void clear_linked_list(LinkedList* list);
void free_linked_list(LinkedList* list);
void add_node(LinkedList* list, void* data);
void remove_node(LinkedList* list, Node* node);
void* get_node(LinkedList* list, size_t index);
void* get_node_data(LinkedList* list, size_t index);
size_t get_node_index(LinkedList* list, Node* node);
size_t linked_list_size(LinkedList* list);
LinkedListIterator* create_linked_list_iterator(LinkedList* list);
void free_linked_list_iterator(LinkedListIterator* iterator);
bool iterator_has_next(LinkedListIterator* iterator);
void* iterator_next_data(LinkedListIterator* iterator);
void iterator_remove(LinkedListIterator* iterator);
