/*
 * @Author: your name
 * @Date: 2021-06-03 12:56:25
 * @LastEditTime: 2021-06-05 23:56:10
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \malloc\memlib.h
 */
#include <unistd.h>

void mem_init(void);
void mem_deinit(void);
void *mem_sbrk(int incr);
void mem_reset_brk(void);
void *mem_heap_lo(void);
void *mem_heap_hi(void);
size_t mem_heapsize(void);
size_t mem_pagesize(void);
