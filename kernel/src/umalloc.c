#include "umalloc.h"

__attribute__ ((section(".kernel"), used)) struct FreeListNode* free_list_node_head = NULL;
__attribute__ ((section(".kernel"), used)) struct FreeListNode* free_list_node_tail = NULL;

static void rearrange_free_list() {
	struct FreeListNode* p = free_list_node_head;
	struct FreeListNode* t = NULL;
	while (p->next != NULL) {
		if (p->address + p->length == p->next->address) {
			p->length += p->next->length;
			t = p->next;
			p->next = p->next->next;
			free(t);
			continue;
		}

		p = p->next;
	}
}

static void delete_free_list_node(struct FreeListNode* pre, struct FreeListNode* p) {
	if (p == free_list_node_head) {
		free_list_node_head = free_list_node_head->next;
	} else {
		pre->next = p->next;
	}

	free(p);
}

uint32_t allocate_memory(uint32_t size) {
	struct FreeListNode* p = free_list_node_head;
	struct FreeListNode *choose = NULL, *pre = NULL, *pre_choose = NULL;
	struct MemoryBlock *result = NULL;
	int mb_size = sizeof(struct MemoryBlock);
	size = (size + 3) / 4 * 4;

	while (p != NULL) {
		if (p->length >= size) {
			if (choose == NULL || choose->length > p->length) {
				pre_choose = pre;
				choose = p;
			}
		}

		pre = p;
		p = p->next;
	}

	if (choose == NULL) {
		return 0;
	}

	result = (struct MemoryBlock*) choose->address;
	result->length = size;
	result->owner = get_current_task()->tid;

	if (choose->length == size) {
		delete_free_list_node(pre_choose, choose);
	} else {
		choose->address = choose->address + size;
		choose->length = choose->length - size;
	}

	add_buffer_entry((uint32_t)result + mb_size, size - mb_size, 0);
//	printf("UMALLOC ALLOCATE MEMORY: %x, size: %d\r\n", (uint32_t)result + mb_size, size - mb_size);
	return (uint32_t) result + mb_size;
}

void change_owner(uint32_t buffer_start_address, int owner) {
	struct MemoryBlock *memory_block = (struct MemoryBlock*)buffer_start_address;
	--memory_block;
	memory_block->owner = owner;
}

void free_memory(uint32_t address) {
	struct FreeListNode *node, *p, *pre;
	struct MemoryBlock *memory_block = (struct MemoryBlock*)address;
	--memory_block;
	node = (struct FreeListNode*)malloc(sizeof(struct FreeListNode));
	if (node == NULL) {
		panic("Memory is insufficient.");
	}

	node->address = (uint32_t)memory_block;
	node->length = memory_block->length;
	node->next = NULL;
	remove_buffer_entry(address);
//	printf("UMALLOC: FREE MEMORY: %x\r\n", address);

	p = free_list_node_head;
	if (p == NULL) {
		free_list_node_head = free_list_node_tail = node;
		return;
	}

	pre = NULL;
	while (p != NULL) {
		if (p->address > node->address) {
			if (pre == NULL) {
				node->next = p;
				free_list_node_head = node;
			} else {
				pre->next = node;
				node->next = p;
			}

			break;
		}

		pre = p;
		p = p->next;
	}

	rearrange_free_list();
}

int get_owner(uint32_t buffer_start_address) {
	if (buffer_start_address >= &__user_section_begin && buffer_start_address < &__user_section_end) {
		struct MemoryBlock *memory_block = (struct MemoryBlock*)buffer_start_address;
		--memory_block;
		return memory_block->owner;
	}

	return -1;
}

int has_ownership(uint32_t buffer_start_address) {
	struct MemoryBlock *memory_block = (struct MemoryBlock*)buffer_start_address;
	--memory_block;
	return memory_block->owner == get_current_task()->tid;
}

void init_free_list(uint32_t address, uint32_t length) {
	free_list_node_head = (struct FreeListNode*)malloc(sizeof(struct FreeListNode));
	if (free_list_node_head == NULL) {
		panic("Memory is insufficient.");
	}

	free_list_node_tail = free_list_node_head;
	free_list_node_head->address = address;
	free_list_node_head->length = length;
	free_list_node_head->next = NULL;
}
