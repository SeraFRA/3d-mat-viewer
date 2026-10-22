#include "dynamic_list.h"
#include <stdlib.h>
#include <string.h>

#define DEFAULT_INITIAL_CAPACITY 8
#define GROWTH_FACTOR 2

bool dynamic_list_init(DynamicList *list, size_t element_size, size_t initial_capacity) {
    if (!list || element_size == 0) {
        return false;
    }

    memset(list, 0, sizeof(DynamicList));

    if (initial_capacity == 0) {
        initial_capacity = DEFAULT_INITIAL_CAPACITY;
    }

    list->element_size = element_size;
    list->count = 0;
    list->capacity = initial_capacity;
    list->data = malloc(element_size * initial_capacity);

    return list->data != NULL;
}

bool dynamic_list_push(DynamicList *list, const void *element) {
    if (!list || !element) {
        return false;
    }

    if (list->count >= list->capacity) {
        size_t new_capacity = (list->capacity == 0) ? DEFAULT_INITIAL_CAPACITY : list->capacity * GROWTH_FACTOR;
        void *new_data = realloc(list->data, new_capacity * list->element_size);
        if (!new_data) {
            return false;
        }
        list->data = new_data;
        list->capacity = new_capacity;
    }

    unsigned char *target = (unsigned char *)list->data + (list->count * list->element_size);
    memcpy(target, element, list->element_size);
    list->count++;
    return true;
}

void *dynamic_list_get(const DynamicList *list, size_t index) {
    if (!list || !list->data || index >= list->count) {
        return NULL;
    }
    return (unsigned char *)list->data + (index * list->element_size);
}

void *dynamic_list_peek(const DynamicList *list) {
    if (!list || !list->data || list->count == 0) {
        return NULL;
    }
    return (unsigned char *)list->data + ((list->count - 1) * list->element_size);
}

bool dynamic_list_pop(DynamicList *list, void *out_element) {
    if (!list || !list->data || list->count == 0) {
        return false;
    }

    list->count--;
    if (out_element) {
        unsigned char *source = (unsigned char *)list->data + (list->count * list->element_size);
        memcpy(out_element, source, list->element_size);
    }
    return true;
}

void dynamic_list_clear(DynamicList *list) {
    if (list) {
        list->count = 0;
    }
}

void dynamic_list_free(DynamicList *list) {
    if (list) {
        if (list->data) {
            free(list->data);
            list->data = NULL;
        }
        list->count = 0;
        list->capacity = 0;
        list->element_size = 0;
    }
}
