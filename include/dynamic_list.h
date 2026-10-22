#ifndef DYNAMIC_LIST_H
#define DYNAMIC_LIST_H

#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Dynamic array list that grows automatically.
 * Contiguous memory layout provides optimal cache locality for engine evaluations.
 */
typedef struct {
    void *data;
    size_t element_size;
    size_t count;
    size_t capacity;
} DynamicList;

/**
 * @brief Initialize a dynamic list with an initial capacity.
 */
bool dynamic_list_init(DynamicList *list, size_t element_size, size_t initial_capacity);

/**
 * @brief Push an element to the end of the list.
 */
bool dynamic_list_push(DynamicList *list, const void *element);

/**
 * @brief Get a pointer to the element at the specified index.
 */
void *dynamic_list_get(const DynamicList *list, size_t index);

/**
 * @brief Peek at the last element in the list without removing it.
 */
void *dynamic_list_peek(const DynamicList *list);

/**
 * @brief Pop the last element from the list, optionally copying it to out_element.
 */
bool dynamic_list_pop(DynamicList *list, void *out_element);

/**
 * @brief Clear all elements from the list without deallocating buffer.
 */
void dynamic_list_clear(DynamicList *list);

/**
 * @brief Free resources associated with the dynamic list.
 */
void dynamic_list_free(DynamicList *list);

#endif /* DYNAMIC_LIST_H */
