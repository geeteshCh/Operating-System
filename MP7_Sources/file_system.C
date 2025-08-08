/*
     File        : file_system.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File System class.
                   Has support for numerical file identifiers.
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/
#define MAX_FILE_BLOCKS 64

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file_system.H"

/*--------------------------------------------------------------------------*/
/* CLASS Inode */

Inode::Inode()
{
  file_system = NULL;
  is_inode_free = true;
  file_system_size= 0;
  id = -1;
  #ifdef _BONUS_OPTION
  blocks = nullptr;
  blocks_size = 0;
  #endif
}

/*--------------------------------------------------------------------------*/

/* You may need to add a few functions, for example to help read and store 
   inodes from and to disk. */

/*--------------------------------------------------------------------------*/
/* CLASS FileSystem */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

FileSystem::FileSystem() {
    Console::puts("File System Constructor: Initializing the data structures\n");
    disk = NULL;
    size = 0;
    free_count = SimpleDisk::BLOCK_SIZE;
    free_blocks = new unsigned char[free_count];
    inode_count= 0;
    inodes = new Inode[MAX_INODES];
}

FileSystem::~FileSystem() {
    Console::puts("File System Destructor: Unmounting the file system\n");
    disk->write(0,free_blocks);
    disk->write(1,(unsigned char*) inodes);
}


/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/


bool FileSystem::Mount(SimpleDisk * _disk) {
    Console::puts("Mounting file system onto the disk\n");
    inode_count= 0;
    unsigned char* temp;
    // Reading the first two blocks of disk which contains free blocks and inodes
    disk = _disk;
    _disk->read(0,free_blocks);
    _disk->read(1,temp);
    // Mounting the inodes and setting the inode count
    inodes = (Inode *) temp;
    int i = 0;
    while(i<MAX_INODES&&!inodes[i].is_inode_free){
        i++;
    }
    inode_count = i;

    return true;
}

bool FileSystem::Format(SimpleDisk * _disk, unsigned int _size) { // static!
    Console::puts("Formatting disk\n");
    unsigned char* array = new unsigned char[(_size/SimpleDisk::BLOCK_SIZE)];
    
    //Resetting the status of the blocks to format the disk 
    array[0] = '0';
    array[1] = '0';
    int i =2;
    while(i<(_size/SimpleDisk::BLOCK_SIZE)){
      array[i]='1';
      i++;
    }
    _disk->write(0,array);

    //Creating a new inode array and writing it to the disk
    Inode* temp = new Inode[MAX_INODES];
    unsigned char* temp1 = (unsigned char*) temp;
    _disk->write(1,temp1);
    return true;
}

Inode * FileSystem::LookupFile(int _file_id) {
    Console::puts("Looking up file "); Console::puti(_file_id); Console::puts("\n");

    // Looking for the file in the inode list
    for (int i = 0;i++;i<inode_count){
      if (inodes[i].id==_file_id){
        return &inodes[i];
      }
    }

    //If the file is not present
    assert(false);

}

bool FileSystem::CreateFile(int _file_id) {
    Console::puts("Creating file "); Console::puti(_file_id); Console::puts("\n");
    
    
    
    int free_inode_idx;
    int free_block_idx;
    
    //Finding the first free block and inode
    int i = 0;
    while(i<free_count){
        if (free_blocks[i]=='1'){
            free_block_idx = i;
            break;
        }
        i++;
    }
    i = 0;
    while(i<MAX_INODES){
        if (inodes[i].is_inode_free){
            free_inode_idx = i;
            break;
        }
        i++;
    }
    
    #ifndef _BONUS_OPTION
    //Setting the values of the inode and block according to new created file
    inodes[free_inode_idx].file_system = this;
    inodes[free_inode_idx].id = _file_id;
    inodes[free_inode_idx].is_inode_free = false;
    free_blocks[free_block_idx] = '0';
    inodes[free_inode_idx].block_number = free_block_idx;
    
    #else
    // Allocating and initializing blocks for the new file
    inodes[free_inode_idx].file_system = this;
    inodes[free_inode_idx].id = _file_id;
    inodes[free_inode_idx].blocks = new int[MAX_FILE_BLOCKS];  // Assuming MAX_FILE_BLOCKS is defined as per your system's requirements
    inodes[free_inode_idx].blocks[0] = free_block_idx;  // Assign the first free block
    inodes[free_inode_idx].blocks_count = 1;  // Start with one block

    for (int j = 1; j < MAX_FILE_BLOCKS; j++) {
        inodes[free_inode_idx].blocks[j] = -1;  // Initialize remaining blocks to -1 (indicating unassigned)
    }
    free_blocks[free_block_idx] = '0';
    inodes[free_inode_idx].block_number = free_block_idx;
    #endif
    return true;
}

bool FileSystem::DeleteFile(int _file_id) {
    Console::puts("Deleting file "); Console::puti(_file_id); Console::puts("\n");
    int d_inode_index=-1; 
    int i = 0;
    //Checking if the file is present in the inode list
    while(i<MAX_INODES)
    {
      if (inodes[i].id==_file_id){
        d_inode_index = i;
        //setting found value to true if file_id is found
        break;
      }
      i++;
    }
    //If the file is not present
    assert(d_inode_index!=-1);

    #ifndef _BONUS_OPTION
    //Deleting the file by setting the inode and block values to free
    int block_no = inodes[d_inode_index].block_number;
    free_blocks[block_no] = '1';
    inodes[d_inode_index].file_system_size = 0;
    inodes[d_inode_index].is_inode_free = true;
    #else
    int block_no = inodes[d_inode_index].block_number;
    free_blocks[block_no] = '1';
    inodes[d_inode_index].file_system_size = 0;
    inodes[d_inode_index].is_inode_free = true;
    for (int j = 0; j < inodes[d_inode_index].blocks_count; j++) {
        int block_no = inodes[d_inode_index].blocks[j];
        if (block_no != -1) {  
            free_blocks[block_no] = '1';  
        }
    }
    delete[] inodes[d_inode_index].blocks;  
    inodes[d_inode_index].blocks = nullptr;  
    inodes[d_inode_index].blocks_count = 0;  
   
    #endif
    return true;
}
