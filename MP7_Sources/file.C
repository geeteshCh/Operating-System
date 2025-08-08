/*
     File        : file.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File class, with support for
                   sequential read/write operations.
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file.H"

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR */
/*--------------------------------------------------------------------------*/

File::File(FileSystem *_fs, int _id) {
    Console::puts("File Constructor: Opening the file.\n");
    // Initialize the file variables
    current_position = 0;
    filesystem = _fs;
    file_id = _id;

    // Find the inode for the file
    int max_nodes = filesystem->MAX_INODES;
    for (int i=0 ; i<max_nodes ; i<i++){
        if (filesystem->inodes[i].id==_id){
            block_number = filesystem->inodes[i].block_number;
            this->file_system_size = filesystem->inodes[i].file_system_size;
            inode_idx = i;
        }
    }

    
}

File::~File() {
    Console::puts("File Destructor: Closing the file.\n");
    filesystem->disk->write(block_number,block_cache);
    filesystem->inodes[inode_idx].file_system_size = this->file_system_size;
   
    filesystem->disk->write(1,(unsigned char *)filesystem->inodes);
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char *_buf) {
    Console::puts("Reading from file\n");
    // Read from the file and return the number of bytes read
    int count = 0;
    int i = current_position;
    while(i < this->file_system_size){
        if (count==_n){
            break;
        }
        _buf[count]=block_cache[i];
        count++;
        i++;
    }
    return count;
}

int File::Write(unsigned int _n, const char *_buf) {
    Console::puts("writing to file\n");
    // Write to the file and return the number of bytes written
    int count = 0;
    int i = current_position;
    while(i - current_position < _n){
        if (i == SimpleDisk::BLOCK_SIZE){
            break;
        }

        block_cache[current_position] = _buf[count];
        count++;
        current_position++;
        this->file_system_size++;
        i++;
    }
    return count;
}

void File::Reset() {
    // Reset the current position to the beginning of the file
    current_position = 0;
}

bool File::EoF() {
    Console::puts("checking for EoF\n");
    // Check if the current position is at the end of the file
    if(current_position >= this->file_system_size)
    {
        return true;
    }
    return false;
}
