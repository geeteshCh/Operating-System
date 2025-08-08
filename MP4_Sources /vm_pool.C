/*
 File: vm_pool.C
 
 Author:
 Date  :
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "page_table.H"

#include "vm_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"
#include "machine.H"


#define PAGE_SIZE Machine::PAGE_SIZE
/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   V M P o o l */
/*--------------------------------------------------------------------------*/

VMPool::VMPool(unsigned long  _base_address,
               unsigned long  _size,
               ContFramePool *_frame_pool,
               PageTable     *_page_table) {
    // Initializing the VM pool
    this->base_address = _base_address;
    this->size = _size;
    this->frame_pool = _frame_pool;
    this->page_table = _page_table;

    // Initializing alloc_reg to keep track of the allocated regions
    total_regions = 0;
    alloc_reg = (alloc_vm *) (this->base_address);
    page_table->register_pool(this);
}

unsigned long VMPool::allocate(unsigned long _size) {
    
    unsigned long address;

    //calculating the number of frames required for the given size(ignoring the internal fragmentation)
    unsigned long frames = _size / (PAGE_SIZE) + (_size % (PAGE_SIZE) > 0? 1 : 0);

    //allocating the address based on if the region is the first region or not
    if (total_regions > 0) {
        address = alloc_reg[total_regions - 1].base_address + alloc_reg[total_regions - 1].size ;
    } else {
        address = this->base_address + PAGE_SIZE;
    }

    //updating the alloc_reg with the new region
    alloc_reg[total_regions].base_address  = address;
    alloc_reg[total_regions].size = frames*(PAGE_SIZE);
    total_regions++;

    return address;
}

void VMPool::release(unsigned long _start_address) {
    unsigned long curent_region;

    //finding the region to be released based on the given adress
    for (int i = 0; i < MAX_VM_REGIONS; i++) {
        if (alloc_reg[i].base_address == _start_address) {
            curent_region = i;
        }
    }

    //freeing the pages in the page table for the given region
    for (int i = 0 ; i < (alloc_reg[curent_region].size) >> 12 ;i++) {
        page_table->free_page(_start_address + i*PAGE_SIZE);
    }

    //updating the alloc_reg after releasing the region
    for (int i = curent_region; i < total_regions - 1; i++) {
        alloc_reg[i] = alloc_reg[i+1];
    }
    total_regions--;

    //Loading the page table to flush the TLB
    page_table->load();

}

bool VMPool::is_legitimate(unsigned long _address) {
    // Checking whether the address is in the valid range of not
    if ((_address < this->base_address + size) && (_address >= this->base_address))
            return true;
    else{
        return false;
    }
 
}

