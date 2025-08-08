/*
 File: page_table.H
 
 Author: R. Bettati
 Department of Computer Science
 Texas A&M University
 Date  : 17/12/07
 Update: 21/10/13
 
 Description: Basic Paging.
 
 */

#ifndef _page_table_H_                   // include file only once
#define _page_table_H_

// SYSTEM CONSTANTS ACCORDING TO CURRENT MEMORY SIZE AND PAGE TABLE LAYOUT
// CAN BE CHANGED TO ADAPT TO DIFFERENT MEMORY SIZES
// USE BIT  => To verify if the PDE or PTE is valid or not
// WRITE BIT => To verify if the page is being written or not
// ADDR_SHIFT_PAGE_DIRECTORY => Used to extract the index in the page directory
// ADDR_SHIFT_PAGE_TABLE_PAGE => Used to extract the index in the page table page
// PAGE_TABLE_PAGE_MASK => Also used to extract the index in the page table page
// PAGE_DIRECTORY_FRAME_SIZE => Size of the page directory in number of frames
// PAGE_TABLE_PAGE_FRAME_SIZE => Size of the page table page in number of frames
// PAGE_DIRECTORY_ENTRY_MASK => Used to extract page table address from the page directory entry
#define USE_BIT 0x1
#define WRITE_BIT 0x2
#define ADDR_SHIFT_PAGE_DIRECTORY 22
#define ADDR_SHIFT_PAGE_TABLE_PAGE 12
#define PAGE_TABLE_PAGE_MASK 0x003ff
#define PAGE_DIRECTORY_FRAME_SIZE 0x1
#define PAGE_TABLE_PAGE_FRAME_SIZE 0x1
#define PAGE_DIRECTORY_ENTRY_MASK 0xfffff000


/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "machine.H"
#include "exceptions.H"
#include "cont_frame_pool.H"
#include "vm_pool.H"

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* Forward declaration of class VMPool */
/* We need this to break a circular include sequence. */
class VMPool;

/*--------------------------------------------------------------------------*/
/* P A G E - T A B L E  */
/*--------------------------------------------------------------------------*/

class PageTable {
    
private:
    
    
    /* THESE MEMBERS ARE COMMON TO ENTIRE PAGING SUBSYSTEM */
    static PageTable     * current_page_table; /* pointer to currently loaded page table object */
    static unsigned int    paging_enabled;     /* is paging turned on (i.e. are addresses logical)? */
    static ContFramePool * kernel_mem_pool;    /* Frame pool for the kernel memory */
    static ContFramePool * process_mem_pool;   /* Frame pool for the process memory */
    static unsigned long   shared_size;        /* size of shared address space */
    
    /* DATA FOR CURRENT PAGE TABLE */
    //To keep track of VM pool that is linked with the page table
    VMPool *   vm_pool_register[VM_POOL_SIZE];
    unsigned int vm_pool_number;


    unsigned long        * page_directory;     /* where is page directory located? */
    
public:
    static const unsigned int PAGE_SIZE        = Machine::PAGE_SIZE;
    /* in bytes */
    static const unsigned int ENTRIES_PER_PAGE = Machine::PT_ENTRIES_PER_PAGE;
    /* in entries */
    
    static void init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size);
    /* Set the global parameters for the paging subsystem. */
    
    PageTable();
    /* Initializes a page table with a given location for the directory and the
     page table proper.
     NOTE: The PageTable object still needs to be stored somewhere!
     Probably it is best to have it on the stack, as there is no
     memory manager yet...
     NOTE2: It may also be simpler to create the first page table *before*
     paging has been enabled.
     */
    
    void load();
    /* Makes the given page table the current table. This must be done once during
     system startup and whenever the address space is switched (e.g. during
     process switching). */
    
    static void enable_paging();
    /* Enable paging on the CPU. Typically, a CPU start with paging disabled, and
     memory is accessed by addressing physical memory directly. After paging is
     enabled, memory is addressed logically. */
    
    static void handle_fault(REGS * _r);
    /* The page fault handler. */
    
    // -- NEW IN MP4
    
    void register_pool(VMPool * _vm_pool);
    /* Register a virtual memory pool with the page table. */
    
    void free_page(unsigned long _page_no);
    /* If page is valid, release frame and mark page invalid. */

    unsigned long * PTE_address(unsigned long addr);
    
};

#endif

