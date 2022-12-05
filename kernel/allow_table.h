#ifndef __H_ALLOW_TABLE_H_
#define __H_ALLOW_TABLE_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "jsw_rbtree.h"

struct allow_table_entry {
	uint32_t address;
	uint32_t length;
};

typedef struct jsw_rbtree allow_table;

void allow_table_new();
int allow_table_insert(uint32_t address, uint32_t length);
int allow_table_erase(uint32_t address);
struct allow_table_entry* allow_table_lower_bound(uint32_t address);
struct allow_table_entry* allow_table_reverse_lower_bound(uint32_t address);

#endif /* __H_ALLOW_TABLE_H_ */
