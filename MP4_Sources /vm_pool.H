/*
    File: vm_pool.H

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 10/26/2010
    Update: 21/10/13

    Description: Management of the Virtual Memory Pool


*/

#ifndef _VM_POOL_H_                   // include file only once
#define _VM_POOL_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */



// Number of VM regisions is 512 because we can only keep track of 512 regions wih one frame
#define MAX_VM_REGIONS 512
#define VM_POOL_SIZE 10

// struct alloc_reg_ {
//     unsigned long base_address;
//     unsigned long size;
// };
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "utils.H"
#include "cont_frame_pool.H"
#include "page_table.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* Forward declaration of class PageTable */
/* We need this to break a circular include sequence. */
class PageTable;

/*--------------------------------------------------------------------------*/
/* V M  P o o l  */
/*--------------------------------------------------------------------------*/

class VMPool { /* Virtual Memory Pool */
private:
   /* -- DEFINE YOUR VIRTUAL MEMORY POOL DATA STRUCTURE(s) HERE. */

    // Every VM pool has a base address, size, and is linked to a frame pool and page table
    unsigned long   base_address;
    unsigned long   size;
    ContFramePool  *frame_pool;
    PageTable      *page_table;

    //To keep track of the allocated regions
    class alloc_vm
    {
      public:
          unsigned long base_address;
          unsigned long size;
    };
    alloc_vm *alloc_reg;
    unsigned int total_regions;

    

public:
   VMPool(unsigned long  _base_address,
          unsigned long  _size,
          ContFramePool *_frame_pool,
          PageTable     *_page_table);
   /* Initializes the data structures needed for the management of this
    * virtual-memory pool.
    * _base_address is the logical start address of the pool.
    * _size is the size of the pool in bytes.
    * _frame_pool points to the frame pool that provides the virtual
    * memory pool with physical memory frames.
    * _page_table points to the page table that maps the logical memory
    * references to physical addresses. */

   unsigned long allocate(unsigned long _size);
   /* Allocates a region of _size bytes of memory from the virtual
    * memory pool. If successful, returns the virtual address of the
    * start of the allocated region of memory. If fails, returns 0. */

   void release(unsigned long _start_address);
   /* Releases a region of previously allocated memory. The region
    * is identified by its start address, which was returned when the
    * region was allocated. */

   bool is_legitimate(unsigned long _address);
   /* Returns false if the address is not valid. An address is not valid
    * if it is not part of a region that is currently allocated. */

 };

#endif
