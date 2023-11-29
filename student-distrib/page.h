#ifndef PAGE_H
#define PAGE_H

#ifndef ASM

#include "types.h"

#define ALIGN           4096        // alignment
#define PAGE_SIZE       1024        // total entry size of each page
#define VIDEO_ADDR      0xB8000     // from lib.c
#define KERNEL_ADDR     0x400000    // from documentation (Appendix C)
#define USER_ADDR_INDEX 32

#define shift_12    12
#define shift_22    22

/* loads page directory */
extern void loadPageDirectory(unsigned int*);
/* enables paging */
extern void enablePaging();
/* flushes TLBs */
extern void flushTLB();
/* initializes paging */
extern void init_page();

/* kb page directory entry structure */
typedef struct __attribute__((packed)) page_directory_entry_kb {
    uint8_t present             : 1;
    uint8_t read_write          : 1;
    uint8_t user_supervisor     : 1;
    uint8_t write_through       : 1;
    uint8_t cache_disabled      : 1;
    uint8_t accessed            : 1;
    uint8_t reserved            : 1;
    uint8_t page_size           : 1;
    uint8_t global              : 1;
    uint8_t avail               : 3;    // bit size: 3
    uint32_t base_addr          : 20;   // bit size: 20
} page_directory_entry_kb_t;

/* mb page directory entry structure */
typedef struct __attribute__((packed)) page_directory_entry_mb {
    uint8_t present             : 1;
    uint8_t read_write          : 1;
    uint8_t user_supervisor     : 1;
    uint8_t write_through       : 1;
    uint8_t cache_disabled      : 1;
    uint8_t accessed            : 1;
    uint8_t dirty               : 1;
    uint8_t page_size           : 1;
    uint8_t global              : 1;
    uint8_t avail               : 3;    // bit size: 3
    uint8_t attr_idx            : 1;
    uint16_t reserved           : 9;    // bit size: 9
    uint16_t base_addr          : 10;   // bit size: 10
} page_directory_entry_mb_t;

/* page directory entry structure */
typedef union page_directory_entry {
    struct page_directory_entry_kb kb;
    struct page_directory_entry_mb mb;
} page_directory_entry_t;

/* page table entry structure */
typedef struct __attribute__((packed)) page_table_entry {
    uint8_t present             : 1;
    uint8_t read_write          : 1;
    uint8_t user_supervisor     : 1;
    uint8_t write_through       : 1;
    uint8_t cache_disabled      : 1;
    uint8_t accessed            : 1;
    uint8_t dirty               : 1;
    uint8_t attr_idx            : 1;
    uint8_t global              : 1;
    uint8_t avail               : 3;    // bit size: 3
    uint32_t base_addr          : 20;   // bit size: 20
} page_table_entry_t;

/* page directory */
page_directory_entry_t page_directory[PAGE_SIZE] __attribute__ ((aligned(ALIGN)));
/* page table */
page_table_entry_t page_table[PAGE_SIZE] __attribute__ ((aligned(ALIGN)));
page_table_entry_t vid_map[PAGE_SIZE] __attribute__ ((aligned(ALIGN)));

#endif /* ASM */

#endif /* PAGE_H */

