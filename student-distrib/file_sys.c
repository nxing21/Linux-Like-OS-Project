#include "file_sys.h"
#include "lib.h"

boot_block_t *boot_block;
inode_t * inode;
uint32_t data_blocks;

void init_file_sys(uint32_t starting_addr){
    boot_block= (boot_block_t *) starting_addr;
    inode = (inode_t *)(starting_addr + BYTES_PER_BLOCK); // starting inode address
    data_blocks = (starting_addr + BYTES_PER_BLOCK + boot_block->inode_count * BYTES_PER_BLOCK); // starting data blocks address
}

int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry){
    dentry_t * dentries_array = boot_block->direntries;
    int i;
    int len= strlen((int8_t *)fname);
    if(len > FILENAME_LEN){
        for(i = 0; i < DIR_ENTRIES; i++){
            //strncmp assumes same length
            const int8_t* cur_dentry = (const int8_t*) dentries_array[i].filename;
            if( (strncmp((int8_t *)cur_dentry, (int8_t *)fname, FILENAME_LEN) == 0)){
                *dentry = dentries_array[i];
                return 0;
            }
        }
    }
    else{
        for(i = 0; i < DIR_ENTRIES; i++){
            //strncmp assumes same length
            const int8_t* cur_dentry = (const int8_t*) dentries_array[i].filename;
            if( (len == strlen((int8_t *)cur_dentry))  && (strncmp((int8_t *)cur_dentry, (int8_t *)fname, len) == 0)){
                *dentry = dentries_array[i];
                return 0;
            }
        }
    }

    return -1; // not found
}


int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){
    dentry_t * dentries_array = boot_block->direntries;
    uint32_t num_dentry = boot_block->dir_count;

    if (index < num_dentry) {
        *dentry = dentries_array[index];
        return 0;
    } 

    return -1; // not found
}


/*The last routine works much like the read system call, reading up to
length bytes starting from position offset in the file with inode number inode and returning the number of bytes
read and placed in the buffer. A return value of 0 thus indicates that the end of the file has been reached.
*/
int32_t read_data (uint32_t inode_num, uint32_t offset, uint8_t* buf, uint32_t length){
    // clear();
    uint32_t i; // loop counter

    /* Fail cases:
     * 1) Inode input is greater than the number of inodes we have
     * 2) Offset is greater than the length of our inode
     */
    if (inode_num >= boot_block->inode_count) {
        return -1;
    }

    uint32_t inode_block_index = offset / BYTES_PER_BLOCK; // current data block index within inode
    uint32_t data_block_index = offset % BYTES_PER_BLOCK; // index in data block

    int32_t num_bytes_copied = 0; // bytes copied counter
    inode_t * cur_inode = (inode_t*) ((uint32_t) inode + inode_num * BYTES_PER_BLOCK); // get current inode
    if (offset >= cur_inode->length) {
        return -1;
    }
    // Change the length if we will be going over the last data block in the current inode
    if (length + offset > cur_inode->length) {
        length = cur_inode->length - offset;
    }
    
    for (i = 0; i < length; i++) {
        uint8_t * src = (uint8_t *) (data_blocks + cur_inode->data_block_num[inode_block_index] * BYTES_PER_BLOCK + data_block_index); // copy into buffer
        memcpy(buf + num_bytes_copied, src, 1);
        num_bytes_copied++;
        data_block_index++;

        /* If we reached the end of the current data block, go to the next one */
        if (data_block_index >= BYTES_PER_BLOCK) {
            data_block_index = 0;
            inode_block_index++;
        }
    }

    return num_bytes_copied;
}

int32_t read_file(int32_t fd, void* buf, int32_t nbytes) {
    uint8_t* fname = "frame1.txt"; // Edit this line for a different file
    dentry_t dentry;
    read_dentry_by_name((const uint8_t *) fname, &dentry);
	
    uint32_t inode_number = dentry.inode_num;

    uint8_t buffer[4 * BYTES_PER_BLOCK]; // 4 is an arbitrary number, we just wanted a large size
	int32_t bytes_read;
	bytes_read = read_data(inode_number, 0, buffer, 4 * BYTES_PER_BLOCK); // 4 is an arbitrary number, we just wanted a large size
	clear();
    printf("\n \n");
	int i;
	for (i = 0; i < bytes_read; i++) {
		printf("%c", buffer[i]);
	}

    return 0;
}
int32_t write_file(int32_t fd, const void* buf, int32_t nbytes) {
    // do nothing
    return -1;
}
int32_t open_file(const uint8_t* filename){
    dentry_t dentry;
    return read_dentry_by_name(filename, &dentry);
}
int32_t close_file(int32_t fd) {
    // do nothing
    return 0;
}

int32_t read_directory(int32_t fd, void* buf, int32_t nbytes) {
    clear();
    printf("\n \n");
    uint32_t i;
    uint32_t j;
    for (i = 0; i < boot_block->dir_count; i++) {
        printf("File Name: ");
        dentry_t dentry = boot_block->direntries[i];
        read_dentry_by_index(i, &dentry);
        for (j = 0; j < FILENAME_LEN; j++) {
            putc(dentry.filename[j]);
        }
        printf(", File Type: ");
        printf("%d", dentry.filetype);
        printf(", File Size: ");
        uint32_t inode_number = dentry.inode_num;
        inode_t * cur_inode = (inode_t*) ((uint32_t) inode + inode_number * BYTES_PER_BLOCK); // get current inode
        printf("%d", cur_inode->length);
        printf("\n");
    }

    return 0;
}
int32_t write_directory(int32_t fd, const void* buf, int32_t nbytes) {
    // do nothing
    return -1;
}
int32_t open_directory(const uint8_t* filename) {
    dentry_t dentry;
    return read_dentry_by_name(filename, &dentry);
}
int32_t close_directory(int32_t fd) {
    // do nothing
    return 0;
}

