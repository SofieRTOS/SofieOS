#ifndef __H_LINKED_LIST_H_
#define __H_LINKED_LIST_H_
#include <stdio.h>

struct LinkedListNode {
	void* data;
	struct LinkedListNode* next;
};

struct LinkedList {
	struct LinkedListNode* head;
	struct LinkedListNode* tail;
};

void init_linked_list(struct LinkedList* list);
void add_list_node(struct LinkedList* list, struct LinkedListNode* node);

#endif /* __H_LINKED_LIST_H_ */
