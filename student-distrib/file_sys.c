#include "file_sys.h"


void init_file_sys(){
    
    int i;

    for(i = 0; i < DIR_ENTRIES; i++){
        file_system.boot_block.direntries[i] = 
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
    dentry_t * dentries_array = file_system.boot_block.direntries;
    int i;
    for(i = 0; i < DIR_ENTRIES; i++){
        if(dentries_array[i].filename == fname)
    }
}

int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){

}

int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){

}
