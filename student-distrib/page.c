#include "page.h"

#define shift_12   12

void init_page() {
    unsigned int i, mem;

    // filling in blank page directory
    for (i = 0; i < PAGE_SIZE; i++) {
        page_directory[i].base_addr = 0;
        page_directory[i].avail = 0;
        page_directory[i].global = 0;
        page_directory[i].page_size = 1;   // 4 MB pages
        page_directory[i].reserved = 0;
        page_directory[i].accessed = 0;
        page_directory[i].cache_disabled = 0;
        page_directory[i].write_through = 0;
        page_directory[i].user_supervisor = 0;
        page_directory[i].read_write = 1;   // enables read/write
        page_directory[i].present = 0;
    }

    // setup page_directory[0]
    page_directory[0].base_addr = (unsigned int)(page_table) >> shift_12;
    page_directory[0].present = 1;   // present
    page_directory[0].page_size = 0;   // 4 kB pages

    // setup page_directory[1] -- kernel memory
    page_directory[1].base_addr = (KERNEL_ADDR >> shift_12);
    page_directory[1].present = 1;
    page_directory[1].page_size = 1;

    // filling in page table
    for (i = 0; i < PAGE_SIZE; i++) {
        page_table[i].base_addr = i;
        page_table[i].avail = 0;
        page_table[i].global = 0;
        page_table[i].attr_idx = 0;
        page_table[i].dirty = 0;
        page_table[i].accessed = 0;
        page_table[i].cache_disabled = 0;
        page_table[i].write_through = 0;
        page_table[i].user_supervisor = 0;
        page_table[i].read_write = 1;
        page_table[i].present = 0;
    }

    mem = VIDEO_ADDR / ALIGN;

    page_table[mem].present = 1;
    page_table[mem].cache_disabled = 1;

    // page_table[mem].base_addr = (unsigned int)(video_mem) >> shift_12;

    // for (i = 0; i < PAGE_SIZE; i++) {
    //     video_mem[i].base_addr = i;
    //     video_mem[i].avail = 0;
    //     video_mem[i].global = 0;
    //     video_mem[i].attr_idx = 0;
    //     video_mem[i].dirty = 0;
    //     video_mem[i].accessed = 0;
    //     video_mem[i].cache_disabled = 0;
    //     video_mem[i].write_through = 0;
    //     video_mem[i].user_supervisor = 0;
    //     video_mem[i].read_write = 1;
    //     video_mem[i].present = 1;
    // }
    // page_table[mem].present = 1;

    loadPageDirectory((unsigned int*)page_directory);
    enablePaging();
}

