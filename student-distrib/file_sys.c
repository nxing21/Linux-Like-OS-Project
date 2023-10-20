#include "file_sys.h"
#include "lib.h"

boot_block_t *boot_block;

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
    int len= strlen((int8_t *)fname);
    dentry_t found_dentry;
    if(len > FILENAME_LEN){
        return -1;
    }
    for(i = 0; i < DIR_ENTRIES; i++){
        //strncmp assumes same length
        const int8_t* cur_dentry = (const int8_t*) dentries_array[i].filename;
        if( (len == strlen((int8_t *)cur_dentry))  && (strncmp((int8_t *)cur_dentry, (int8_t *)fname, len) == 0)){
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
    inode_t * inode = (inode_t *)(boot_block + BYTES_PER_BLOCK); // starting inode address
    uint32_t * data_blocks = (uint32_t *) (inode + inode->length * BYTES_PER_BLOCK); // starting data blocks address
    int i; // loop counter

    /* Fail cases:
     * 1) Inode input is greater than the number of inodes we have
     * 2) Offset is greater than the length of our inode
     */
    if (inode_num >= boot_block->inode_count) {
        return -1;
    }
    if (offset >= inode->length) {
        return -1;
    }

    uint32_t inode_block_index = offset / BYTES_PER_BLOCK; // current data block index within inode
    uint32_t data_block_index = offset % BYTES_PER_BLOCK; // index in data block

    int32_t num_bytes_copied = 0; // bytes copied counter
    inode_t * cur_inode = (inode_t*) (data_blocks + inode_num * BYTES_PER_BLOCK); // get current inode

    // Change the length if we will be going over the last data block in the current inode
    if (offset + length > cur_inode->length) {
        length = cur_inode->length - offset;
    }

    for (i = 0; i < length; i++) {
        uint32_t * cur_block = (uint32_t *) (cur_inode->data_block_num[inode_block_index]); // get current data block

        buf[num_bytes_copied] = cur_block[data_block_index]; // copy into buffer

        // update counters and index trackers
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
