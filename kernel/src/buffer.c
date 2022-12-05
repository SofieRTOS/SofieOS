#include "buffer.h"
#include "pmp.h"
#include "util.h"

__attribute__ ((section(".kernel"), used)) struct BufferEntry *buffer_entries_head = NULL;

void add_buffer_entry(uint32_t address, uint32_t length, uint8_t owner) {
	struct BufferEntry *new_entry = (struct BufferEntry*)malloc(sizeof(struct BufferEntry));
	if (new_entry == NULL) {
		panic("Memory is insufficient.");
	}

	struct BufferEntry *pre = NULL;
	struct BufferEntry *p = buffer_entries_head;
	while (p != NULL && p->address <= address) {
		if (p->address == address) {
			free(new_entry);
			return;
		}

		pre = p;
		p = p->next;
	}

	new_entry->address = address;
	new_entry->length = length;
	new_entry->owner = owner;

	if (pre == NULL) {
		new_entry->next = buffer_entries_head;
		buffer_entries_head = new_entry;
	} else {
		pre->next = new_entry;
		new_entry->next = p;
	}

	enable_buffer_access(address, address+length, 0);
}

void clear_buffer_entries(uint8_t task_id) {
	struct BufferEntry* p = buffer_entries_head;
	struct BufferEntry* pre = NULL;
	while (p) {
		if (p->owner == task_id) {
			if (pre == NULL) {
				buffer_entries_head = p->next;
			} else {
				pre->next = p->next;
			}
			free(p);
			if (pre == NULL) {
				p = buffer_entries_head;
			} else {
				p = pre->next;
			}
			continue;
		}

		pre = p;
		p = p->next;
	}
}

void remove_buffer_entry(uint32_t address) {
	struct BufferEntry *p = buffer_entries_head;
	struct BufferEntry *pre = NULL;
	while (p != NULL) {
		if (p->address == address) {
			if (p == buffer_entries_head) {
				buffer_entries_head = p->next;
			} else {
				pre->next = p->next;
			}

			free(p);
			break;
		}

		pre = p;
		p = p->next;
	}

	if (p) clear_buffer_access(address);
}
