#include "page.h"


/*
 * init_page
 *   DESCRIPTION: initializes paging
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: initializes paging
 */   
void init_page() {
    unsigned int i;   // looping variable
    unsigned int mem;   // index of video memory in page table

    // filling in blank page directory
    for (i = 0; i < PAGE_SIZE; i++) {
        if (i != 0) {
            page_directory[i].mb.present = 0;   // default: not present
            page_directory[i].mb.read_write = 1;   // enables read/write
            page_directory[i].mb.user_supervisor = 0;
            page_directory[i].mb.write_through = 0;
            page_directory[i].mb.cache_disabled = 0;
            page_directory[i].mb.accessed = 0;
            page_directory[i].mb.dirty = 0;
            page_directory[i].mb.page_size = 1;   // default: 4 MB pages
            page_directory[i].mb.global = 0;
            page_directory[i].mb.avail = 0;
            page_directory[i].mb.attr_idx = 0;
            page_directory[i].mb.reserved = 0;
            page_directory[i].mb.base_addr = 0;
        }
    }

    // setup page_directory[0]
    page_directory[0].kb.present = 1;   // present
    page_directory[0].kb.read_write = 1;   // enables read/write
    page_directory[0].kb.user_supervisor = 0;
    page_directory[0].kb.write_through = 0;
    page_directory[0].kb.cache_disabled = 0;
    page_directory[0].kb.accessed = 0;
    page_directory[0].kb.reserved = 0;
    page_directory[0].kb.page_size = 0;   // 4 kB pages
    page_directory[0].kb.global = 0;
    page_directory[0].kb.avail = 0;
    page_directory[0].kb.base_addr = (unsigned int)(page_table) >> shift_12;

    // setup page_directory[1] -- kernel memory
    page_directory[1].mb.present = 1;   // present
    page_directory[1].mb.base_addr = (unsigned int)(KERNEL_ADDR) >> shift_22;

    // filling in page table
    for (i = 0; i < PAGE_SIZE; i++) {
        page_table[i].present = 0;   // default: not present
        page_table[i].read_write = 1;   // enables read/write
        page_table[i].user_supervisor = 0;
        page_table[i].write_through = 0;
        page_table[i].cache_disabled = 0;
        page_table[i].accessed = 0;
        page_table[i].dirty = 0;
        page_table[i].attr_idx = 0;
        page_table[i].global = 0;
        page_table[i].avail = 0;
        page_table[i].base_addr = i;

        vid_map[i].present = 0;   // default: not present
        vid_map[i].read_write = 1;   // enables read/write
        vid_map[i].user_supervisor = 0;
        vid_map[i].write_through = 0;
        vid_map[i].cache_disabled = 0;
        vid_map[i].accessed = 0;
        vid_map[i].dirty = 0;
        vid_map[i].attr_idx = 0;
        vid_map[i].global = 0;
        vid_map[i].avail = 0;
        vid_map[i].base_addr = i;
    }

    mem = VIDEO_ADDR / ALIGN;   // gets index of video memory in page table

    /*Video Page for Screen*/
    page_table[mem].present = 1;   // present

    /*Video Page for Terminal 0*/
    page_table[mem+1].present = 1;   // present
    
    /*Video Page for Terminal 1*/
    page_table[mem+2].present = 1;   // present

    /*Video Page for Terminal 2*/
    page_table[mem+3].present = 1;   // present



    
    // Assembly functions to set up paging
    loadPageDirectory((unsigned int*)page_directory);
    enablePaging();
}


