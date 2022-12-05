#ifndef __UMALLOC_H_
#define __UMALLOC_H_

#include <stdlib.h>
#include "buffer.h"
#include "macros.h"
#include "task.h"
#include "util.h"

struct MemoryBlock {
	size_t length;
	int owner;
};

struct FreeListNode {
	uint32_t address;
	uint32_t length;
	struct FreeListNode* next;
};

uint32_t allocate_memory(uint32_t size);
void change_owner(uint32_t buffer_start_address, int owner);
void free_memory(uint32_t address);
int get_owner(uint32_t buffer_start_address);
int has_ownership(uint32_t buffer_start_address);
void init_free_list(uint32_t address, uint32_t length);

#endif
