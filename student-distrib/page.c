#include "page.h"

void init_page() {
    unsigned int i;

    // filling in page directory
    for (i = 0; i < PAGE_SIZE; i++) {
        page_directory[i] = 0x00000002;   // value from osdev
    }

    // filling in page table
    for (i = 0; i < PAGE_SIZE; i++) {
        page_table[i] = (i * 0x1000) | 3;   // value from osdev
    }

    page_directory[0] = ((unsigned_int)page_table) | 3;

    loadPageDirectory(page_directory);
    enablePaging();
}

