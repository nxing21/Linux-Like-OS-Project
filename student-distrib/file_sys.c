#include "file_sys.h"


void init_file_sys(uint32_t starting_addr){
    
    boot_block= (boot_block_t *) starting_addr;
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
    int found_flag = 0;
    int len= strlen(fname);
    dentry_t found_dentry;
    if(len > FILENAME_LEN){
        return -1;
    }
    for(i = 0; i < DIR_ENTRIES; i++){
        //strncmp assumes same length
        const int8_t* curr_dentry = (const int8_t*) dentries_array[i].filename;
        if( (len == strlen(curr_dentry))  && (strncmp(curr_dentry, fname, len) == 0)){
            found_flag = 1;
            found_dentry = dentries_array[i];
            break;
        }
    }

    if(found_flag == 1){
        *dentry = found_dentry;
        return 0;
    }

    return -1; // not found
    
}

int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){
    dentry_t * dentries_array = boot_block->direntries;
    
    // dentry_t found_dentry;
    // uint32_t temp_inode_num;
    uint32_t num_dentry = boot_block->dir_count;

    if( index < num_dentry)
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
    inode_t * inode = (inode_t *)(boot_block + BYTES_PER_BLOCK);
    uint32_t * data_blocks = (uint32_t *) (inode + inode->length * BYTES_PER_BLOCK);
    int i; // loop counter
    if (inode_num >= boot_block->inode_count) {
        return -1;
    }
    if (offset >= inode->length) {
        return -1;
    }
    uint32_t starting_block = offset / BYTES_PER_BLOCK;
    uint32_t starting_block_index = offset % BYTES_PER_BLOCK;

    uint32_t num_bytes_copied = 0;
    uint32_t * cur_block = data_blocks + starting_block * BYTES_PER_BLOCK;

    /**
     * This is the case where the length of the data is all within one data block.
    */
    if (starting_block_index + length < BYTES_PER_BLOCK) {
        for (i = starting_block_index; i < starting_block_index + length; i++) {
            buf[num_bytes_copied] = cur_block[i];
            num_bytes_copied++;
        }

        return num_bytes_copied;
    }

    /**
     * Copy the data within the first block
    */
    for (i = starting_block_index; i < BYTES_PER_BLOCK; i++) {
        buf[num_bytes_copied] = cur_block[i];
        num_bytes_copied++;
    }

    uint32_t ending_block = (offset + length - 1) / BYTES_PER_BLOCK;
    int j; // block counter
    cur_block += BYTES_PER_BLOCK;
    /**
     * This is copying all blocks except first and last
    */
    for (j = starting_block + 1; j <= ending_block - 1; j++) {
        for (i = 0; i < BYTES_PER_BLOCK; i++) {
            buf[num_bytes_copied] = cur_block[i];
            num_bytes_copied++;
        }
        cur_block += BYTES_PER_BLOCK;
    }

    /**
     * This is copying all blocks from last block
    */
    uint32_t num_bytes_last_block = (offset + length) % BYTES_PER_BLOCK;
    for (i = 0; i < num_bytes_last_block; i++) {
        buf[num_bytes_copied] = cur_block[i];
        num_bytes_copied++;
    }

    return num_bytes_copied;
}
