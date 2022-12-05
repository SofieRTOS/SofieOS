#include "buffer.h"
#include "pmp.h"
#include "util.h"
#include "task.h"

static int pmp_robin = 0;

void dump_pmp(int region_id) {
	size_t address;
	struct metal_pmp_config config;
	struct metal_pmp *pmp = metal_pmp_get_device();
	int rc = metal_pmp_get_region(pmp, region_id, &config, &address);
	if (rc) {
		panic("metal_pmp_get_region failed.\r\n");
	}

	printf("[DUMP(%d)->%x] .A = %d, .L = %d, .R = %d, .W = %d, .X = %d\r\n",
			region_id, address << 2, config.A, config.L, config.R, config.W, config.X);
}

void pmp_initialization() {
	int rc;
	struct metal_pmp *pmp;
	/* Initialize PMPs */
	pmp = metal_pmp_get_device();
	if(!pmp) {
		panic("Unable to get PMP Device\n");
	}
	metal_pmp_init(pmp);

	/* Configure PMP 0 to allow access to all memory */
	struct metal_pmp_config config = {
		.L = METAL_PMP_UNLOCKED,
		.A = METAL_PMP_TOR,
		.X = 1,
		.W = 1,
		.R = 1,
	};

	uint32_t address = &metal_segment_stack_begin;
	address >>= 2;
	rc = metal_pmp_set_region(pmp, ALL_REGION_END, config, address);
	if(rc != 0) {
		panic("Failed to configure PMP ALL_END_REGION\n");
	}

	pmp_robin = 0;
}

static void set_top_of_range_pmp_registers(
		uint32_t start_addr, uint32_t end_addr,
		int access, int start_region, int end_region) {
	struct metal_pmp *pmp = metal_pmp_get_device();
	if (!pmp) {
		panic("Unable to get PMP Device\n");
	}

	struct metal_pmp_config config = {
		.L = METAL_PMP_UNLOCKED,
		.A = METAL_PMP_OFF,
		.X = 0,
		.W = access,
		.R = access,
	};

	int rc;

	rc = metal_pmp_set_region(pmp, end_region, config, end_addr >> 2);
	if (rc) {
		panic("Failed to configure PMP");
	}

	rc = metal_pmp_set_region(pmp, start_region, config, start_addr >> 2);
	if (rc) {
		panic("Failed to configure PMP");
	}

	rc = metal_pmp_set_address_mode(pmp, end_region, METAL_PMP_TOR);
	if (rc) {
		panic("Failed to configure PMP");
	}
}

void clear_buffer_access(uint32_t address) {
	struct metal_pmp *pmp = metal_pmp_get_device();
	uint32_t start_address = metal_pmp_get_address(pmp, DYNAMIC_ALLOWED_REGION_START) << 2;
	if (start_address >= address) {
		metal_pmp_set_address_mode(pmp, DYNAMIC_ALLOWED_REGION_END, METAL_PMP_OFF);
		metal_pmp_set_address_mode(pmp, DYNAMIC_ALLOWED_REGION_START, METAL_PMP_OFF);
	}

	start_address = metal_pmp_get_address(pmp, HEAP_REGION_START) << 2;
	if (start_address >= address) {
		metal_pmp_set_address_mode(pmp, HEAP_REGION_START, METAL_PMP_OFF);
		metal_pmp_set_address_mode(pmp, HEAP_REGION_END, METAL_PMP_OFF);
	}

	start_address = metal_pmp_get_address(pmp, STACK_CONTROL_REGION_START) << 2;
	if (start_address >= address) {
		metal_pmp_set_address_mode(pmp, STACK_CONTROL_REGION_START, METAL_PMP_OFF);
		metal_pmp_set_address_mode(pmp, STACK_CONTROL_REGION_END, METAL_PMP_OFF);
	}

}

void enable_buffer_access(uint32_t start_addr, uint32_t end_addr, uint32_t mode) {
	static int stack_pmp = -1;
	int current = pmp_robin;

	if (stack_pmp != -1) {
		current = stack_pmp;
	} else {
		current = pmp_robin;
	}

	if (current == 0) {
		set_top_of_range_pmp_registers(
			start_addr, end_addr,
			1,
			DYNAMIC_ALLOWED_REGION_START,
			DYNAMIC_ALLOWED_REGION_END);
			pmp_robin = 1;
			if (mode) stack_pmp = 0;
	}
	else if (current == 1) {
		set_top_of_range_pmp_registers(
			start_addr, end_addr,
			1,
			HEAP_REGION_START,
			HEAP_REGION_END);
			pmp_robin = 2;
			if (mode) stack_pmp = 1;
	}
	else if (current == 2) {
		set_top_of_range_pmp_registers(
			start_addr, end_addr,
			1,
			STACK_CONTROL_REGION_START,
			STACK_CONTROL_REGION_END);
			pmp_robin = 0;
			if (mode) stack_pmp = 2;
	}

	if (stack_pmp != -1 && mode == 0) {
		stack_pmp = -1;
	}
}

void log_pmp_settings(struct PmpSettings* settings) {
	struct metal_pmp_config config;
	struct metal_pmp *pmp = metal_pmp_get_device();
	int i;
	settings->enabled = 0;
	for (i = 0; i < 6; i++) {
		metal_pmp_get_region(pmp, i, &config, &settings->addr[i]);
		if (config.A) {
			settings->enabled |= (1<<i);
		}
	}

	settings->pmp_robin = pmp_robin;
}

void restore_pmp_settings(struct PmpSettings* settings) {
	struct metal_pmp *pmp = metal_pmp_get_device();
	int i;
	struct metal_pmp_config config = {
		.L = METAL_PMP_UNLOCKED,
		.A = METAL_PMP_OFF,
		.X = 0,
		.W = 1,
		.R = 1,
	};

	for (i = 0; i < 6; i++) {
		if ((settings->enabled >> i)&1) {
			config.A = METAL_PMP_TOR;
		} else {
			config.A = METAL_PMP_OFF;
		}

		metal_pmp_set_region(pmp, i, config, settings->addr[i]);
	}

	pmp_robin = settings->pmp_robin;
}
