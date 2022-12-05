#ifndef __H_PMP_H_
#define __H_PMP_H_
#include <stdint.h>
#include <metal/pmp.h>

#define DYNAMIC_ALLOWED_REGION_START  0
#define DYNAMIC_ALLOWED_REGION_END    1
#define HEAP_REGION_START             2
#define HEAP_REGION_END               3
#define STACK_CONTROL_REGION_START    4
#define STACK_CONTROL_REGION_END      5
#define ALL_REGION_START              6
#define ALL_REGION_END                7

struct PmpSettings{
	uint32_t addr[6];
	int enabled;
	int pmp_robin;
};

void pmp_initialization();

//////////////////////////////////////////////////////
void dump_pmp(int region_id);
void clear_buffer_access(uint32_t address);
void enable_buffer_access(uint32_t start_addr, uint32_t end_addr, uint32_t mode);
void log_pmp_settings(struct PmpSettings* settings);
void restore_pmp_settings(struct PmpSettings* settings);
//////////////////////////////////////////////////////

#endif /* __H_PMP_H_ */
