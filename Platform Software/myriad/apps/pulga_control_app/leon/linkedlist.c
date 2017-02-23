/*!
    \file
 
    \brief     Library for implementing linkedlists
*/


#include "linkedlist.h"
#include <stdlib.h>
#include <string.h>

#define SUCCESS 1
#define FAIL 0

#define COMP_CALLBACK int (*comparator)(void *a, void *b)

#ifdef DEBUG
    #define DEBUG_OUTPUT(string) (printf("[debug] %s\n",string))
#else
    #define DEBUG_OUTPUT(string)
#endif

/* PRIVATE */

/*!
 *  \brief Search item in the list in position index starting from the first element
 *  \param[in]      start item
 *  \param[in]      index of the item
 *  \return         Item in the list in position index starting from the first element
*/
void *get_item_fwd(item *list, unsigned int index) {
    if (list == NULL) {
        return NULL;
    }
    if (index == 0) {
        return list->value;
    }
    return get_item_fwd(list->next, --index);
}


/*!
 *  \brief Search item in the list in position index starting from the last element
 *  \param[in]      start item
 *  \param[in]      index of the item
 *  \return         Item in the list in position index starting from the last element
*/
void *get_item_rev(item *list, unsigned int index) {
    if (list == NULL) {
        return NULL;
    }
    if (index == 0) {
        return list->value;
    }
    return get_item_rev(list->prev, --index);
}

/*!
 *  \brief Determine if an item exists in a list
 *  \param[in]      current item compared
 *  \param[in]      value of the item searched
 *  \param[in]      comparator function for items
 *  \return         1 if the item is found, 0 if not
*/
int item_exists(item *list, void *value, int (*comparator)(void *a, void *b)) {
    if (list == NULL) {
        return 0;
    }
    if (comparator(value, list->value) == 0) {
        return 1;
    }
    return item_exists(list->next, value, comparator);
}

/*!
 *  \brief Determine the position of an item exists in a list
 *  \param[in]      current item compared
 *  \param[in]      value of the item searched
 *  \param[in]      comparator function for items
 *  \param[in]      current position evaluated
 *  \return         pos value if the item is found, -1 if not
*/
int item_position(item *list, void *value, int (*comparator)(void *a, void *b), int pos) {
    if (list == NULL) {
        return -1;
    }
    if (comparator(value, list->value) == 0) {
        return pos;
    }
    return item_position(list->next, value, comparator,pos+1);
}

/*!
 *  \brief Remove the first ocurrence of an item in the list
 *  \param[in]      current item compared
 *  \param[in]      value of the item searched
 *  \param[in]      comparator function for items
 *  \return         1 if the item is found, 0 if not
*/
int item_remove(item *list, void *value, int (*comparator)(void *a, void *b)) {
    if (list == NULL) {
        return 0;
    }
    if (comparator(value, list->value) == 0) {
		//it is necessary to free the list->value
		//free(temp);

		list->next->prev=list->prev;
		*list=*list->next;
		
        return 1;
    }
    return item_remove(list->next, value, comparator);
}

/*!
 *  \brief Sort the list using bubble sort algorithm (ascending)
 *  \param[in]      list to order
 *  \param[in]      comparator function for items
 *  \return         head of the ordered list
*/
item *bubble_sort(item *list, int (*comparator)(void *a, void *b)) {
    item *head = list;
    register int swapped;
    do {
        list = head;
        swapped = 0;
        while (list->next) {
            if (comparator(list->value, list->next->value) > 0) {
                void *temp = list->value;
                list->value = list->next->value;
                list->next->value = temp;
                swapped = 1;
            }
            list = list->next;
        }
    } while (swapped);
    return head;
}

/*!
 *  \brief Sort the list using bubble sort algorithm (ascending)
 *  \param[in]      list to order
 *  \param[in]      comparator function for items
 *  \return         ordered list, NULL if the list as no item
*/
item *sort_list(item *list, int (*comparator)(void *a, void *b)) {
    if (list == NULL) {
        return NULL;
    }
    return bubble_sort(list, comparator);
}

/*!
 *  \brief Clear the list from memory
 *  \param[in]      list to clear
 *  \param[in]      method for clear the list
 *  \return         SUCCESS if the list is cleared
*/
int clear_list(item *list, void (*free_value)(void *)) {
    if (list == NULL) {
        return SUCCESS;
    }
    clear_list(list->next, free_value);
    list->next = NULL;
    if (free_value != NULL) {
        free_value(list->value);
    }
    else {
        free(list->value);
    }
    list->value = NULL;
    free(list);
    //DEBUG_OUTPUT("item removed");
    return SUCCESS;
}

/*!
 *  \brief Print the elements of a list
 *  \param[in]      list/item to print
 *  \param[in,out]  output in which the list is printed
 *  \param[in]      method for printing the value of the item
 *  \param[in]      method for determining if the item is visible
*/
void print_list(item *list, FILE *output, void to_string(FILE *,void *), int (*is_visible)(void *)) {
    if (list != NULL) {
        if (is_visible == NULL || is_visible(list->value)) {
            to_string(output, list->value);
        }
        print_list(list->next, output, to_string, is_visible);
    }
}

/* PUBLIC */

/*!
 *  \brief Create empty list
 *  \return         List created
*/
linkedlist *ll_create() {
    linkedlist *list = malloc(sizeof(linkedlist));
    if (list == NULL) {
        fprintf(stderr, "Error: could not allocate memory for linkedlist\n");
        exit(EXIT_FAILURE);
    }
    list->head = NULL;
    list->leaf = NULL;
    list->current = NULL;
    list->size = 0;
    //DEBUG_OUTPUT("linkedlist created");
    return list;
}

/*!
 *  \brief Push element in the first position of a list
 *  \param[in,out]  list to modify
 *  \param[in]      item to include in the first position
 *  \return         SUCCESS if the element is pushed correctly, EXIT_FAILURE if not
*/
int ll_push_first(linkedlist *list, void *value) {
    item *temp = malloc(sizeof(item));
    if (temp == NULL) {
        fprintf(stderr, "Error: could not allocate memory for item\n");
        exit(EXIT_FAILURE);
    }
    temp->value = value;
    temp->prev = NULL;
    temp->next = list->head;
    if (list->head == NULL) {
        list->leaf = temp;
    }
    else {
        list->head->prev = temp;
    }
    list->head = temp;
    list->size += 1;
    return SUCCESS;
}

/*!
 *  \brief Push element in the last position of a list
 *  \param[in,out]  list to modify
 *  \param[in]      item to include in the last position
 *  \return         SUCCESS if the element is pushed correctly, EXIT_FAILURE if not
*/
int ll_push_last(linkedlist *list, void *value) {
    item *temp = malloc(sizeof(item));
    if (temp == NULL) {
        fprintf(stderr, "Error: could not allocate memory for item\n");
        exit(EXIT_FAILURE);
    }
    temp->value = value;
    temp->prev = list->leaf;
    temp->next = NULL;
    if (list->head == NULL) {
        list->head = temp;
        list->leaf = temp;
    }
    else {
        list->leaf->next = temp;
        list->leaf = temp;
    }
    list->size += 1;
    return SUCCESS;
}

/*!
 *  \brief Obtain first element of the list
 *  \param[in]      list
 *  \return         First element of the list
*/
void *ll_get_first(linkedlist *list) {
    return list->head->value;
}

/*!
 *  \brief Obtain last element of the list
 *  \param[in]      list
 *  \return         Last element of the list
*/
void *ll_get_last(linkedlist *list) {
    return list->leaf->value;
}

/*!
 *  \brief Obtain first element of the list and delete it from the list
 *  \param[in,out]  list
 *  \return         First element of the list
*/
void *ll_pop_first(linkedlist *list) {
    item *temp = list->head;
	void *value;
    if (temp == NULL) {
        //DEBUG_OUTPUT("nothing to pop from beginning of list");
        return NULL;
    }
    list->head = temp->next;
    if (list->head != NULL) {
        list->head->prev = NULL;
    }
    value = temp->value;
    free(temp);
    list->size--;
    //DEBUG_OUTPUT("first item popped from list");
    return value;
}

/*!
 *  \brief Obtain last element of the list and delete it from the list
 *  \param[in,out]  list
 *  \return         Last element of the list
*/
void *ll_pop_last(linkedlist *list) {
    item *temp = list->leaf;
	void *value;
    if (temp == NULL) {
        //DEBUG_OUTPUT("nothing to pop from end of list");
        return NULL;
    }
    list->leaf = temp->prev;
    if (list->leaf != NULL) {
        list->leaf->next = NULL;
    }
    value = temp->value;
    free(temp);
    list->size--;
    //DEBUG_OUTPUT("last item popped from list");
    return value;
}

/*!
 *  \brief Remove all elements of the list
 *  \param[in,out]  list
 *  \return         1 if finished
*/
int ll_pop_all(linkedlist *list) {
    while(list->size>0){
        ll_pop_first(list);
    }
    return 1;
}



/*!
 *  \brief Obtain the element situated on index position on the list
 *  \param[in]      list
 *  \param[in]      index of the element
 *  \return         Index element of the list
*/
void *ll_get_index(linkedlist *list, unsigned int index) {
    if (index >= list->size) {
        fprintf(stderr, "Warning: index out of bounds\n");
        return NULL;
    }
    if (index < list->size / 2) {
        //DEBUG_OUTPUT("get item by index, starting from head");
        return get_item_fwd(list->head, index);
    }
    else {
        //DEBUG_OUTPUT("get item by index, starting from leaf");
        return get_item_rev(list->leaf, list->size - 1 - index);
    }
}

/*!
 *  \brief Obtain the next element of the list
 *  \param[in]      list
 *  \return         Next element of the list
*/
void *ll_get_next(linkedlist *list) {
    if (list->current == NULL) {
        if (list->head == NULL) {
            return NULL;
        }
        else {
            list->current = list->head;
            return list->current->value;
        }
    }
    else {
        list->current = list->current->next;
        if (list->current == NULL) {
            return NULL;
        }
        else {
            return list->current->value;
        }
    }
}

/*!
 *  \brief Obtain the previous element of the list
 *  \param[in]      list
 *  \return         Previous element of the list
*/
void *ll_get_prev(linkedlist *list) {
    if (list->current == NULL) {
        if (list->leaf == NULL) {
            return NULL;
        }
        else {
            list->current = list->leaf;
            return list->current->value;
        }
    }
    else {
        list->current = list->current->prev;
        if (list->current == NULL) {
            return NULL;
        }
        else {
            return list->current->value;
        }
    }
}

/*!
 *  \brief Test if an item exists in the list
 *  \param[in]      list
 *  \param[in]      item to be searched
 *  \param[in]      comparator function for items
 *  \return         1 if exists, 0 if not
*/
int ll_exists(linkedlist *list, void *value, int (*comparator)(void *, void *)) {
    return item_exists(list->head, value, comparator);
}

/*!
 *  \brief Obtain the position of an item in the list
 *  \param[in]      list
 *  \param[in]      item to be searched
 *  \param[in]      comparator function for items
 *  \return         position of the item if found, -1 if not
*/
int ll_item_position(linkedlist *list, void *value, int (*comparator)(void *, void *)) {
    return item_position(list->head, value, comparator, 0);
}

/*!
 *  \brief Remove an item from the list
 *  \param[in]      list
 *  \param[in]      item to be removed
 *  \param[in]      comparator function for items
 *  \return         1 if removed, 0 if not
*/
int ll_remove_item(linkedlist *list, void *value, int (*comparator)(void *, void *)) {
	int result=0;
	if(comparator(list->head->value,value)==0){
		ll_pop_first(list);
		result=1;
	}
	else if(comparator(list->leaf->value,value)==0){
		ll_pop_last(list);
		result=1;
	}
	else{
		result=item_remove(list->head, value, comparator);
		if(result==1)
			list->size--;
	}
	return result;
}

/*!
 *  \brief Sort list (ascending)
 *  \param[in]      list
 *  \param[in]      comparator function for items
 *  \return         SUCCESS if sorted, FAIL if not (1 and 0)
*/
int ll_sort(linkedlist *list, int (*comparator)(void *a, void *b)) {
    if (comparator == NULL) {
        fprintf(stderr, "Warning: comparator function is NULL, cannot sort\n");
        return FAIL;
    }
    else {
        list->head = sort_list(list->head, comparator);
        //DEBUG_OUTPUT("linkedlist sorted");
        return SUCCESS;
    }
}

/*!
 *  \brief Clear the list from memory
 *  \param[in]      list to clear
 *  \param[in]      method for clear the list
 *  \return         SUCCESS if the list is cleared
*/
int ll_clear(linkedlist *list, void (*free_value)(void *)) {
    if (list->head != NULL) {
        clear_list(list->head, free_value);
        list->head = NULL;
    }
    list->size = 0;
    //DEBUG_OUTPUT("linkedlist cleared");
    return SUCCESS;
}

/*!
 *  \brief Delete the list from memory
 *  \param[in]      list to clear
 *  \param[in]      method for clear the list
 *  \return         SUCCESS if the list is deleted
*/
int ll_delete(linkedlist *list, void (*free_value)(void *)) {
    ll_clear(list, free_value);
    list->head = NULL;
    free(list);
    //DEBUG_OUTPUT("linkedlist deleted");
    return SUCCESS;
}

/*!
 *  \brief Print the elements of a list
 *  \param[in]      list/item to print
 *  \param[in,out]  output in which the list is printevid
 *  \param[in]      method for printing the value of the item
*/
void ll_print(linkedlist *list, FILE *output, void (*to_string)(FILE *, void *)) {
    print_list(list->head, output, to_string, NULL);
}

/*!
 *  \brief Print the elements of a list using a filter
 *  \param[in]      list/item to print
 *  \param[in,out]  output in which the list is printed
 *  \param[in]      method for printing the value of the item
 *  \param[in]      method for the filter
*/
void ll_print_filter(linkedlist *list, FILE *output, void (*to_string)(FILE *, void *), int (*is_visible)(void *)) {
    print_list(list->head, output, to_string, is_visible);
}

/*!
 *  \brief Dump and print the complete information of a list
 *  \param[in]      list/item to print
 *  \param[in,out]  output in which the list is printed
 *  \param[in]      method for printing the value of the item
*/
void ll_dump(linkedlist *list, FILE *output, void (*to_string)(FILE *, void *)) {
    fprintf(output, "\n### linkedlist dump ############################################################\n\n");
    fprintf(output, " Version: %s\n", LL_VERSION);
    fprintf(output, " Head: %lX\n", (long)list->head);
    fprintf(output, " Leaf: %lX\n", (long)list->leaf);
    fprintf(output, " Current: %lX\n", (long)list->current);
    fprintf(output, " Items: %d\n", list->size);
    fprintf(output, "\n################################################################################\n\n");
    if (list->size > 0) {
        item *next = list->head;
        unsigned int index = 0;
        while (next != NULL) {
            fprintf(output, " index %d: %-8.lX prev: %-8.lX next: %-8.lX ", index++, (long)next, (long)next->prev, (long)next->next);
            if (to_string != NULL) {
                to_string(output, next->value);
            }
            else {
                fprintf(output, "value: %-8.lX", (long)next->value);
            }
            fprintf(output, "\n");
            next = next->next;
        }
    }
    else {
        fprintf(output, " (no items)\n");
    }
    fprintf(output, "\n################################################################################\n\n");
}