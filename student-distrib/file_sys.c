#include "file_sys.h"
#include "lib.h"

boot_block_t *boot_block;
inode_t * inode;
uint32_t * data_blocks;
dentry_t *dentry;

void init_file_sys(uint32_t starting_addr){
    boot_block= (boot_block_t *) starting_addr;
    inode = (inode_t *)(starting_addr + BYTES_PER_BLOCK); // starting inode address
    data_blocks = (uint32_t *) (starting_addr + BYTES_PER_BLOCK + boot_block->inode_count * BYTES_PER_BLOCK); // starting data blocks address
    // file_descriptors[0] = stdin; /*keyboard input*/
    // file_descriptors[1] = stdout; /*terminal output*/
}

/* The three routines provided by the file system module return -1 on failure, indicating a non-existent file or invalid
* index in the case of the first two calls, or an invalid inode number in the case of the last routine. Note that the directory
* entries are indexed starting with 0. Also note that the read_data call can only check that the given inode is within the
* valid range. It does not check that the inode actually corresponds to a file (not all inodes are used). However, if a bad
* data block number is found within the file bounds of the given inode, the function should also return -1.
*
* When successful, the first two calls fill in the dentry_t block passed as their second argument with the file name, file
* type, and inode number for the file, then return 0. The last routine works much like the read system call, reading up to
* length bytes starting from position offset in the file with inode number inode and returning the number of bytes
* read and placed in the buffer. A return value of 0 thus indicates that the end of the file has been reached. 
*/

int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry){
    dentry_t * dentries_array = boot_block->direntries;
    int i;
    int len = strlen((int8_t *)fname);
    dentry_t found_dentry;
    if(len > FILENAME_LEN){
        return -1;
    }
    for(i = 0; i < DIR_ENTRIES; i++){
        //strncmp assumes same length
        int8_t* cur_dentry = (int8_t*) dentries_array[i].filename;
        if ((len == strlen((int8_t *)cur_dentry)) && (strncmp((int8_t *)cur_dentry, (int8_t *)fname, len) == 0)){
            read_dentry_by_index(i, dentry);
            return 0;
        }
    }

    return -1; // not found

    
}

int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){
    dentry_t * dentries_array = boot_block->direntries;
    
    // dentry_t found_dentry;
    // uint32_t temp_inode_num;
    uint32_t num_dentry = boot_block->dir_count;

    if (index < num_dentry)
    {
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
    // printf("reached line 77\n");
    // //inode_t * inode2 = (inode_t *)(&boot_block + BYTES_PER_BLOCK); // starting inode address
    // printf("reached line 79\n");
    // //uint32_t * data_blocks2 = (uint32_t *) (&inode + boot_block->inode_count * BYTES_PER_BLOCK); // starting data blocks address
    // printf("reached line 81\n");
    clear();
    printf("1: %d, %d, %d\n", boot_block, inode, data_blocks);
    //printf("2: %d, %d, %d\n", boot_block, inode2, data_blocks2);
    uint32_t i; // loop counter
    printf("reached line 83\n");
    printf("%d\n", boot_block->inode_count);

    /* Fail cases:
     * 1) Inode input is greater than the number of inodes we have
     * 2) Offset is greater than the length of our inode
     */
    if (inode_num >= boot_block->inode_count) {
        return -1;
    }
    if (offset >= boot_block->inode_count) {
        return -1;
    }

    uint32_t inode_block_index = offset / BYTES_PER_BLOCK; // current data block index within inode
    uint32_t data_block_index = offset % BYTES_PER_BLOCK; // index in data block

    int32_t num_bytes_copied = 0; // bytes copied counter
    inode_t * cur_inode = (inode_t*) ((uint32_t) inode + inode_num * BYTES_PER_BLOCK); // get current inode
    printf("reached line 104\n");
    printf("%d, %d, %d, %d, %d, %d, %d\n", boot_block, inode, cur_inode->length, boot_block->inode_count, cur_inode->data_block_num[0], cur_inode->data_block_num[1],cur_inode->data_block_num[2]);
    // Change the length if we will be going over the last data block in the current inode
    // if (offset + length > cur_inode->length) {
    //     length = cur_inode->length - offset;
    // }
    // printf("dfsafdfa %d\n", &inode[inode_num].data_block_num[0]);
    printf("reached line 110, length = %d\n", length);

    for (i = 0; i < length; i++) {
        //printf("reached line 113, i = %d\n", i);
        // uint32_t cur_block_num = cur_inode->data_block_num[inode_block_index]; // get current data block
        // //printf("%d, %d\n", boot_block->data_count, cur_block_num);
        // uint32_t * cur_block_addr = data_blocks + cur_block_num * BYTES_PER_BLOCK;
        
        buf[num_bytes_copied] = (uint8_t *) (data_blocks + cur_inode->data_block_num[inode_block_index] * BYTES_PER_BLOCK + data_block_index); // copy into buffer
        //printf("reached line 117, i = %d\n", i);

        // update counters and index trackers
        num_bytes_copied++;
        data_block_index++;

        /* If we reached the end of the current data block, go to the next one */
        if (data_block_index >= BYTES_PER_BLOCK) {
            data_block_index = 0;
            inode_block_index++;
        }
    }
    printf("reached line 139\n");

    return num_bytes_copied;
}

int32_t read_file(int32_t fd, void* buf, int32_t nbytes) {
    
}
int32_t write_file(int32_t fd, const void* buf, int32_t nbytes) {
    // do nothing
    return -1;
}
int32_t open_file(const uint8_t* filename){
    return read_dentry_by_name(filename, dentry);
}
int32_t close_file(int32_t fd) {
    // do nothing
    return 0;
}

int32_t read_directory(int32_t fd, void* buf, int32_t nbytes) {
    
}
int32_t write_directory(int32_t fd, const void* buf, int32_t nbytes) {
    // do nothing
    return -1;
}
int32_t open_directory(const uint8_t* filename) {
    return read_dentry_by_name(filename, dentry);
}
int32_t close_directory(int32_t fd) {
    // do nothing
    return 0;
}

