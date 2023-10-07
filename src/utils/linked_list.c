#include "linked_list.h"
static void free_node_data_func(void* data){
    free(data);
}
LinkedList* create_linked_list(){
    LinkedList *list = malloc(sizeof(LinkedList));
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    list->free_data = free_node_data_func;
    return list;
}

LinkedList* create_linked_list_with_free(void (*free_data)(void*)){
    LinkedList *list = malloc(sizeof(LinkedList));
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    list->free_data = free_data;
    return list;
}

void clear_linked_list(LinkedList* list){
    if(list == NULL){
        return;
    }
    Node* current = list->head;
    while(current != NULL){
        Node* next = current->next;
        list->free_data(current->data);
        free(current);
        current = next;
    }
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

void free_linked_list(LinkedList* list){
    if(list == NULL){
        return;
    }
    clear_linked_list(list);
    free(list);
}

void linked_list_add_node(LinkedList* list, void* data){
    if(list == NULL){
        return;
    }
    Node* node = malloc(sizeof(Node));
    node->data = data;
    node->next = NULL;
    if(list->head == NULL){
        list->head = node;
    }
    else {
        list->tail->next = node;
    }
    list->tail = node;
    list->size++;
}

void linked_list_remove_node(LinkedList* list, Node* node){
    if(list == NULL || node == NULL){
        return;
    }
    if(list->head == node){
        list->head = node->next;
    }
    else {
        Node* current = list->head;
        while(current != NULL && current->next != node){
            current = current->next;
        }
        if(current != NULL){
            current->next = node->next;
        }
    }
    if(list->tail == node){
        list->tail = node->next;
    }
    list->free_data(node->data);
    free(node);
    list->size--;
}

void* linked_list_get_node(LinkedList* list, size_t index){
    if(list == NULL || index >= list->size){
        return NULL;
    }
    Node* current = list->head;
    for(size_t i = 0; i < index; i++){
        current = current->next;
    }
    return current;
}

void* linked_list_peek_head(LinkedList* list){
    if(list == NULL || list->head == NULL){
        return NULL;
    }
    return list->head->data;
}

void* linked_list_peek_tail(LinkedList* list){
    if(list == NULL || list->tail == NULL){
        return NULL;
    }
    return list->tail->data;
}

void* get_node_data(LinkedList* list, size_t index){
    if(list == NULL || index >= list->size){
        return NULL;
    }
    Node* current = list->head;
    for(size_t i = 0; i < index; i++){
        current = current->next;
    }
    return current->data;
}

size_t get_node_index(LinkedList* list, Node* node){
    if(list == NULL || node == NULL){
        return -1;
    }
    Node* current = list->head;
    size_t index = 0;
    while(current != NULL && current != node){
        current = current->next;
        index++;
    }
    if(current == NULL){
        return -1;
    }
    return index;
}

size_t linked_list_size(LinkedList* list){
    if(list == NULL){
        return 0;
    }
    return list->size;
}

LinkedListIterator* create_linked_list_iterator(LinkedList* list){
    if(list == NULL){
        return NULL;
    }
    LinkedListIterator* iterator = malloc(sizeof(LinkedListIterator));
    iterator->list = list;
    iterator->prev = NULL;
    iterator->current = NULL;
    iterator->next = iterator->list->head;
    return iterator;
}

void free_linked_list_iterator(LinkedListIterator* iterator){
    if(iterator == NULL){
        return;
    }
    free(iterator);
}

bool iterator_has_next(LinkedListIterator* iterator){
    if(iterator == NULL){
        return false;
    }
    return iterator->next != NULL;
}

void* iterator_next_data(LinkedListIterator* iterator){
    if(!iterator_has_next(iterator)){
        return NULL;
    }
    iterator->prev = iterator->current;
    iterator->current = iterator->next;
    iterator->next = iterator->current == NULL ? NULL : iterator->current->next;
    return iterator->current->data;
}

void iterator_remove(LinkedListIterator* iterator){
    if(iterator == NULL || iterator->current == NULL){
        return;
    }
    if(iterator->prev == NULL){
        iterator->list->head = iterator->next;
    }
    else {
        iterator->prev->next = iterator->next;
    }
    if(iterator->next == NULL){
        iterator->list->tail = iterator->prev;
    }
    iterator->list->free_data(iterator->current->data);
    free(iterator->current);
    iterator->current = NULL;
    iterator->list->size--;
}