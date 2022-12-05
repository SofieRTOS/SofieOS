#include "linkedlist.h"

void init_linked_list(struct LinkedList* list) {
	list->head = list->tail = NULL;
}

void add_list_node(struct LinkedList* list, struct LinkedListNode* node) {
	if (list->head == NULL) {
		list->head = list->tail = node;
	} else {
		list->tail->next = node;
		list->tail = list->tail->next;
	}
}
