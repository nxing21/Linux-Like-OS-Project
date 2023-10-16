#ifndef PAGE_H
#define PAGE_H

#ifndef ASM

#include "types.h"

#define ALIGN           4096
#define PAGE_SIZE       1024
#define VIDEO_ADDR      0xB8000    // lib.c
#define KERNEL_ADDR     0x400000   // documentation (Appendix C)


/* Assembly functions to set up paging */
extern void loadPageDirectory(unsigned int*);
extern void enablePaging();
extern void init_page();

/* page directory entry structure */
typedef struct __attribute__((packed, aligned(4))) page_directory_entry {
    uint32_t base_addr          : 20;
    uint8_t avail               : 3;
    uint8_t global              : 1;
    uint8_t page_size           : 1;
    uint8_t reserved            : 1;
    uint8_t accessed            : 1;
    uint8_t cache_disabled      : 1;
    uint8_t write_through       : 1;
    uint8_t user_supervisor     : 1;
    uint8_t read_write          : 1;
    uint8_t present             : 1;
} page_directory_entry_t;

/* page table entry structure */
typedef struct __attribute__((packed, aligned(4))) page_table_entry {
    uint32_t base_addr          : 20;
    uint8_t avail               : 3;
    uint8_t global              : 1;
    uint8_t attr_idx            : 1;
    uint8_t dirty               : 1;
    uint8_t accessed            : 1;
    uint8_t cache_disabled      : 1;
    uint8_t write_through       : 1;
    uint8_t user_supervisor     : 1;
    uint8_t read_write          : 1;
    uint8_t present             : 1;
} page_table_entry_t;

/* page directory */
page_directory_entry_t page_directory[PAGE_SIZE] __attribute__ ((aligned(ALIGN)));
page_table_entry_t page_table[PAGE_SIZE] __attribute__ ((aligned(ALIGN)));
page_table_entry_t video_mem[PAGE_SIZE] __attribute__ ((aligned(ALIGN)));

#endif /* ASM */

#endif /* PAGE_H */

