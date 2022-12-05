#include "allow_table.h"

allow_table *allow_table_v;

static int allow_table_entry_cmp(const void* p1, const void* p2) {
	struct allow_table_entry *entry1, *entry2;
	entry1 = (struct allow_table_entry*) p1;
	entry2 = (struct allow_table_entry*) p2;

	if (entry1->address > entry2->address) {
		return 1;
	} else if (entry1->address < entry2->address) {
		return -1;
	}

	return 0;
}

static void* allow_table_entry_dup(void* p) {
	void *dup_p = malloc(sizeof(struct allow_table_entry));
	memmove(dup_p, p, sizeof(struct allow_table_entry));
	return dup_p;
}

static void allow_table_entry_rel(void* p) {
	free(p);
}

void allow_table_new() {
	allow_table_v = jsw_rbnew(allow_table_entry_cmp, allow_table_entry_dup, allow_table_entry_rel);
}

int allow_table_insert(uint32_t address, uint32_t length) {
	int ret;
	struct allow_table_entry entry;
	entry.address = address;
	entry.length = length;

	ret = jsw_rbinsert(allow_table_v, (void*)&entry);

	if (ret == 0) {
		return -1;
	}

	enable_buffer_access(address, address+length, 0);
	return 0;
}

int allow_table_erase(uint32_t address) {
	int ret;
	struct allow_table_entry entry;
	entry.address = address;
	ret = jsw_rberase(allow_table_v, (void*)&entry);

	if (ret == 0) return -1;

	clear_buffer_access();
	return 0;
}


struct allow_table_entry* allow_table_lower_bound(uint32_t address) {
	struct allow_table_entry entry, *found;
	entry.address = address;
	found = jsw_rblowerbound(allow_table_v, &entry);
	return found;
}

struct allow_table_entry* allow_table_reverse_lower_bound(uint32_t address) {
	struct allow_table_entry entry, *found;
	entry.address = address;
	found = jsw_rbrlowerbound(allow_table_v, &entry);
	return found;
}
