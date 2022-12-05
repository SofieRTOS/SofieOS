#ifndef __H_MACROS_H__
#define __H_MACROS_H__

#define USER_STACK_SIZE        1024
#define KERNEL_STACK_SIZE      768
#define USER_HEAP_SIZE         4096
#define MAX_TASK_COUNT         16

extern unsigned metal_segment_stack_begin;
extern unsigned metal_segment_stack_end;
extern unsigned __user_section_begin;
extern unsigned __user_section_end;
extern unsigned metal_segment_heap_target_start;
extern unsigned metal_segment_heap_target_end;


#endif
