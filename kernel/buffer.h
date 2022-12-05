#ifndef __H_BUFFER_H_
#define __H_BUFFER_H_

#include <stdint.h>
#include <stdlib.h>
#include "macros.h"

struct BufferEntry {
	uint32_t address;
	uint32_t length;
	uint8_t owner;
	struct BufferEntry* next;
};

extern struct BufferEntry *buffer_entries_head;

void add_buffer_entry(uint32_t address, uint32_t length, uint8_t owner);
void clear_buffer_entries(uint8_t task_id);
void remove_buffer_entry(uint32_t address);

#endif /* __H_BUFFER_H_ */
