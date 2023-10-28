#include "syscalls.h"
#include "file_sys.h"
#include "lib.h"

// global variables, keep track of starting index of boot block, inodes, and data blocks
boot_block_t *boot_block;
inode_t * inode;
uint32_t data_blocks;
// global dentry variable
dentry_t *dentry;



/* void init_file_sys(uint32_t starting_addr)
 * Inputs: uint32_t starting_addr = starting address of file system
 * Return Value: none
 * Function: Initializes the file system 
 */
void init_file_sys(uint32_t starting_addr) {
    int i;
    boot_block= (boot_block_t *) starting_addr;
    inode = (inode_t *)(starting_addr + BYTES_PER_BLOCK); // starting inode address
    data_blocks = (starting_addr + BYTES_PER_BLOCK + boot_block->inode_count * BYTES_PER_BLOCK); // starting data blocks address
}

/* int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry)
 * Inputs: const uint8_t* fname: file name of file to be read,
 *         dentry_t* dentry: if file is found this is loaded with the correct dentry
 * Return Value: 0 (success: file found), -1 (fail)
 * Function: Finds file dentry by name
 */
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry) {
    dentry_t * dentries_array = boot_block->direntries;
    int i;
    int found_flag = 0;
    int len= strlen((int8_t *)fname);
    dentry_t found_dentry;
    if(len > FILENAME_LEN) {
        return -1;
    }
    for(i = 0; i < DIR_ENTRIES; i++) {
        //strncmp assumes same length
        const int8_t* cur_dentry = (const int8_t*) dentries_array[i].filename;
        if(strncmp((int8_t *)cur_dentry, (int8_t *)fname, len) == 0) {
            found_flag = 1;
            found_dentry = dentries_array[i];
            break;
        }
    }

    if(found_flag == 1) {
        *dentry = found_dentry;
        dentry->filename[32] = '\0';
        return 0;
    }
    printf("didn't find");
    return -1; // not found
}

/* int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry)
 * Inputs: uint32_t index: dentry array index of file to be read,
 *         dentry_t* dentry: if file is found this is loaded with the correct dentry
 * Return Value: 0 (success: file found), -1 (fail)
 * Function: Finds file dentry by dentry array index
 */
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry) {
    dentry_t * dentries_array = boot_block->direntries;
    uint32_t num_dentry = boot_block->dir_count;

    if(index < num_dentry) {
        *dentry = dentries_array[index];
        return 0;
    } 

    return -1; // not found
}



/* int32_t read_data (uint32_t inode_num, uint32_t offset, uint8_t* buf, uint32_t length)
 * Inputs:  uint32_t inode_num: inode number of file to be read,
 *          uint32_t offset: data offset  in bytes,
 *          uint8_t* buf: char pointer buffer to be filled in by data read,
 *          uint32_t length: length of data to be read up to
 * Return Value: Number of bytes read
 * Function: readS up to length bytes starting from position offset in the file with inode number inode and returning the number of bytes read and placed in the buffer.
 */
int32_t read_data (uint32_t inode_num, uint32_t offset, uint8_t* buf, uint32_t length) {

    uint32_t i; // loop counter

    /* Fail case 1: Inode input is greater than the number of inodes we have. */
    if (inode_num >= boot_block->inode_count) {
        return -1;
    }

    uint32_t inode_block_index = offset / BYTES_PER_BLOCK; // current data block index within inode
    uint32_t data_block_index = offset % BYTES_PER_BLOCK; // index in data block

    int32_t num_bytes_copied = 0; // bytes copied counter
    inode_t * cur_inode = (inode_t*) ((uint32_t) inode + inode_num * BYTES_PER_BLOCK); // get current inode
    /* Fail case 2: Offset is greater than the length of our inode. */
    if (offset >= cur_inode->length) {
        return -1;
    }

    // Change the length if we will be going over the last data block in the current inode
    if (offset + length > cur_inode->length) {
        length = cur_inode->length - offset;
    }
    
    for (i = 0; i < length; i++) {
        // get the address we want and put into src
        uint8_t * src = (uint8_t *) (data_blocks + cur_inode->data_block_num[inode_block_index] * BYTES_PER_BLOCK + data_block_index);
        memcpy(buf + num_bytes_copied, src, 1); // copy 1 byte into buffer
        // iterate counters and index trackers
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



/* int32_t read_file(int32_t fd, void* buf, int32_t nbytes)
 * Inputs:  int32_t fd: file descriptor array index,   
 *          void* buf: void pointer buffer to be filled in by data read,
 *          int32_t nbytes: Number of bytes  to be read
 * Return Value: number of bytes read
 * Function: reads nbytes starting from file position offset in the file using fd index and returning the number of bytes read and placed in the buffer.
 */
int32_t read_file(int32_t fd, void* buf, int32_t nbytes) {
    int offset;
    int32_t bytes_read;
    uint8_t * buffer = (uint8_t*) buf;
    if(fd >= FILE_DESCRIPTOR_MAX || fd < 0) { //check if valid fd index
        return -1;
    }
    else{
        offset = curr_fds[fd].file_pos; //offset based on file position
        curr_fds[fd].file_pos += nbytes; //updating file position
        bytes_read = read_data(curr_fds[fd].inode, offset, buffer, nbytes); // call read data to fill our buffer
        return bytes_read;
    }  
}

/* int32_t write_file(int32_t fd, const void* buf, int32_t nbytes)
 * Inputs:  int32_t fd: file descriptor array index,   
 *          void* buf: void pointer buffer,
 *          int32_t nbytes: Number of bytes  to be written
 * Return Value: -1
 * Function: does nothing since read only file system
 */
int32_t write_file(int32_t fd, const void* buf, int32_t nbytes) {
    // do nothing
    return -1;
}

/* int32_t open_file(const uint8_t* filename)
 * Inputs:  const uint8_t* filename: file name of file to be opened
 * Return Value:  0 (success: file found), -1 (fail)
 * Function: checks if valid name and updates file descriptors accordingly
 */
int32_t open_file(const uint8_t* filename) {
    // dentry_t temp_dentry;
    
    return 0;
}

/* int32_t close_file(int32_t fd) 
 * Inputs: int32_t fd: file desciptor index of file to be closed
 * Return Value:  0 (success: file found), -1 (fail)
 * Function: checks if valid index and updates file descriptors accordingly
 */
int32_t close_file(int32_t fd) {
    curr_fds[fd].flags = -1; //marking as not in use
    curr_fds[fd].inode = -1; //marking as not pointing to any inode
    curr_fds[fd].file_pos = 0; //file position reset to 0 
    return 0;
}

/* int32_t read_directory(int32_t fd, void* buf, void* length_buf, int32_t nbytes)
 * Inputs:  int32_t fd: file descriptor array index, 
 *          void* buf: void pointer buffer to be filled in by data read,
 *          void* length_buf: void pointer length buffer
 *          int32_t nbytes: number of bytes to be read
 * Return Value:  number of bytes read
 * Function: reads directory information
 */
int32_t read_directory(int32_t fd, void* buf, void* length_buf, int32_t nbytes) {
    // loop counters
    uint32_t i;
    uint32_t j;
    // we have two buffers, one for file name and file type, other for file length
    uint8_t * buffer = (uint8_t*) buf;
    uint32_t * length_buffer = (uint32_t *) length_buf;
    // counters so we know what index to put in buffer
    uint32_t num_read = 0;
    uint32_t num_length_read = 0;

    curr_fds[fd].file_pos += nbytes; // updating file position

    for (i = 0; i < boot_block->dir_count; i++) { // iterate through all files
        dentry_t dentry = boot_block->direntries[i];
        // get dentry of current file (index i)
        read_dentry_by_index(i, &dentry);
        for (j = 0; j < FILENAME_LEN; j++) {
            buffer[num_read] = dentry.filename[j]; // copy character in filename into main buffer
            num_read++;
        }
        buffer[num_read] = dentry.filetype + '0'; //changing num to char equivalent
        num_read++;

        uint32_t inode_number = dentry.inode_num;
        inode_t * cur_inode = (inode_t*) ((uint32_t) inode + inode_number * BYTES_PER_BLOCK); // get current inode
        // add length of file into the length buffer
        length_buffer[num_length_read] = cur_inode->length;
        num_length_read++;
    }
    return num_read;
}

/* int32_t write_directory(int32_t fd, const void* buf, int32_t nbytes)
 * Inputs:  int32_t fd: file descriptor array index   
 *          void* buf: void pointer buffer
 *          int32_t nbytes: Number of bytes  to be written
 * Return Value: -1
 * Function: does nothing since read only file system
 */
int32_t write_directory(int32_t fd, const void* buf, int32_t nbytes) {
    // do nothing
    return -1;
}

/* int32_t open_directory(const uint8_t* filename)
 * Inputs:  const uint8_t* filename: file name of directory to be opened
 * Return Value:  0 (success: directory found), -1 (fail)
 * Function: checks if valid name and updates file descriptors accordlingly
 */
int32_t open_directory(const uint8_t* filename) {
    return 0;
}




/* int32_t close_directory(int32_t fd)
 * Inputs: int32_t fd: file desciptor index of directory to be closed
 * Return Value:  0 (success: directory found), -1 (fail)
 * Function: checks if valid index and updates file descriptors accordingly
 */
int32_t close_directory(int32_t fd) {
    curr_fds[fd].flags = -1; //marking as not in use
    curr_fds[fd].inode = -1; //marking as not pointing to any inode
    curr_fds[fd].file_pos = 0; //file position reset to 0 
    return 0;
}


