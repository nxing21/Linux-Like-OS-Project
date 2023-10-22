#include "file_sys.h"
#include "lib.h"

boot_block_t *boot_block;
inode_t * inode;
uint32_t data_blocks;
dentry_t *dentry;

fd_t file_descriptors[8];
uint8_t fds_in_use;

void init_file_sys(uint32_t starting_addr){
    int i;
    boot_block= (boot_block_t *) starting_addr;
    inode = (inode_t *)(starting_addr + BYTES_PER_BLOCK); // starting inode address
    data_blocks = (starting_addr + BYTES_PER_BLOCK + boot_block->inode_count * BYTES_PER_BLOCK); // starting data blocks address


    // file_descriptors[0] = stdin; /*keyboard input*/
    // file_descriptors[1] = stdout; /*terminal output*/
    for(i = 2; i < 8; i++){
        file_descriptors[i].file_op_table_ptr = NULL; // unkown type so set to NULL
        file_descriptors[i].inode = -1; // -1 because it's not set
        file_descriptors[i].file_pos = 0; //default to 0
        file_descriptors[i].flags = -1; // -1 means not in use
        fds_in_use = 2; //easy way to keep track of how many fd slots if any are open
    }
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
    if (offset + length > cur_inode->length) {
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
    int offset;
    int i;
    int32_t bytes_read;
    uint8_t * buffer = (uint8_t*) buf;
    if(fd > 7 || fd < 0){
        return -1;
    }
    else{
        printf("\n \n");
        offset = file_descriptors[fd].file_pos; //the order of me doing this seems wrong
        file_descriptors[fd].file_pos += nbytes;
        bytes_read = read_data(file_descriptors[fd].inode, offset, buffer, nbytes);
        for (i = 0; i < bytes_read; i++) {
		    putc(buffer[i]);
	    }

        return 0;
    }
    
}

int32_t write_file(int32_t fd, const void* buf, int32_t nbytes) {
    // do nothing
    return -1;
}

int32_t open_file(const uint8_t* filename){
    dentry_t temp_dentry;
    
    if( (read_dentry_by_name(filename, &temp_dentry) != -1) && fds_in_use < 8 ){
        file_descriptors[fds_in_use+1].flags = 0x1; 
        file_descriptors[fds_in_use+1].inode = temp_dentry.inode_num;
        fds_in_use++;
        return 0;
    }
    else{
        return -1;
    }
    
    // return exists;
    // return read_dentry_by_name(filename, &temp_dentry);
    
    
}

int32_t close_file(int32_t fd) {
    if(fd >= 0 && fd < 8 ){
        file_descriptors[fd].flags = -1; 
        file_descriptors[fd].inode = -1;
        file_descriptors[fd].file_pos = 0;
        file_descriptors[fd].file_op_table_ptr = NULL;
        fds_in_use--;
        return 0;
    }
    else{
        return -1;
    }
}

int32_t read_directory(int32_t fd, void* buf, int32_t nbytes) {
    int offset;
    uint32_t i;
    uint32_t j;
    uint8_t * buffer = (uint8_t*) buf;

    if(fd > 7 || fd < 0){
        return -1;
    }
    else{
        printf("\n \n");
        offset = file_descriptors[fd].file_pos; //the order of me doing this seems wrong
        file_descriptors[fd].file_pos += nbytes;
        read_data(file_descriptors[fd].inode, offset, buffer, nbytes);

        for (i = 0; i < boot_block->dir_count; i++) {
            printf("File Name: ");
            dentry_t dentry = boot_block->direntries[i];
            read_dentry_by_index(i, &dentry);
            for (j = 0; j < FILENAME_LEN; j++) {
                putc(dentry.filename[j]);
            }
            printf(", File Type: %d, File Size: ", dentry.filetype);

            uint32_t inode_number = dentry.inode_num;
            inode_t * cur_inode = (inode_t*) ((uint32_t) inode + inode_number * BYTES_PER_BLOCK); // get current inode
            printf("%d", cur_inode->length);
            printf("\n");
        }





        return 0;
    }
}

int32_t write_directory(int32_t fd, const void* buf, int32_t nbytes) {
    // do nothing
    return -1;
}
int32_t open_directory(const uint8_t* filename) {
    if(read_dentry_by_name(filename, dentry) != -1 && fds_in_use < 8 ){
        file_descriptors[fds_in_use+1].flags = 1; 
        file_descriptors[fds_in_use+1].inode = dentry->inode_num;
        fds_in_use++;
        return 0;
    }
    else{
        return -1;
    }
}

int32_t close_directory(int32_t fd) {
    if(fd >= 0 && fd < 8 ){
        file_descriptors[fd].flags = -1; 
        file_descriptors[fd].inode = -1;
        file_descriptors[fd].file_pos = 0;
        file_descriptors[fd].file_op_table_ptr = NULL;
        fds_in_use--;
        return 0;
    }
    else{
        return -1;
    }
}

