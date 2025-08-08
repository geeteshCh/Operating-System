#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"


#define PAGE_DIRECTORY_FRAME_SIZE 1


PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool * PageTable::kernel_mem_pool = NULL;
ContFramePool * PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;

void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
   // Setting up the global parameters for the paging subsystem.
   kernel_mem_pool = _kernel_mem_pool;
   process_mem_pool = _process_mem_pool;
   shared_size = _shared_size;

   Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{

//    // Getting a page from the kernel pool and setting it as the page directory
//    page_directory = (unsigned long *) (kernel_mem_pool->get_frames(PAGE_DIRECTORY_FRAME_SIZE)*PAGE_SIZE);

//    // Getting a page from kernel member pool and setting it as the page table page
//    unsigned long * page_table_page = (unsigned long *) (kernel_mem_pool->get_frames(PAGE_TABLE_PAGE_FRAME_SIZE)*PAGE_SIZE);

   // Getting a page from the process pool and setting it as the page directory
   page_directory = (unsigned long *) (process_mem_pool->get_frames(PAGE_DIRECTORY_FRAME_SIZE)*PAGE_SIZE);

   // Getting a page from process member pool and setting it as the page table page
   unsigned long * page_table_page = (unsigned long *) (process_mem_pool->get_frames(PAGE_TABLE_PAGE_FRAME_SIZE)*PAGE_SIZE);


   // INITIALIZATION OF PAGE DIRECTORY
   // Updating the first PDE with the address of the page table page
   page_directory[0] = (unsigned long) page_table_page | WRITE_BIT | USE_BIT ;

   // Setting the rest of the PDEs to 0. Setting it to 0 also sets the use bit of the PDE to 0 indicating that the PDE is inavlid
   for(int i = 1; i < (ENTRIES_PER_PAGE); i++)
   {
      page_directory[i] = 0;
   }
   // Setting the last PDE to point to the page directory itself for recursive page lookup
   page_directory[ENTRIES_PER_PAGE -1] = (unsigned long) (page_directory) | WRITE_BIT | USE_BIT ;


   // DIRECT MAPPING OF FIRST 4MB
   // Mapping the first 4MB of the page table page to the first 4MB of the physical memory
   for(int i = 0; i < (ENTRIES_PER_PAGE); i++)
   {
      page_table_page[i] = (i*PAGE_SIZE) | WRITE_BIT | USE_BIT ;
   }


    // Initializing the Virtual Memory Pool
    for(int i = 0 ; i < VM_POOL_SIZE; i++) {
        vm_pool_register[i] = NULL;
    }
    this->vm_pool_number = 0;
    
}


void PageTable::load()
{
   // Setting the current page table to this page table
   current_page_table = this;

   // Loading the page directory into the CR3 register
   write_cr3((unsigned long) page_directory);
   Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
   // Setting the paging enabled flag to 1 and enabling paging by setting the 31st bit of the CR0 register to 1
   write_cr0(read_cr0() | 0x80000000);
   paging_enabled = 1;
   Console::puts("Enabled Paging\n");
}

void PageTable::handle_fault(REGS * _r)
{

   // Getting a frame from the process pool
   unsigned long frame_addr = process_mem_pool->get_frames(PAGE_DIRECTORY_FRAME_SIZE)*PAGE_SIZE;
   

   // Getting the virtual address which caused the page fault
   unsigned long fault_addr = read_cr2();
   // Getting the page directory index
   unsigned long page_dir_index = fault_addr >> ADDR_SHIFT_PAGE_DIRECTORY;

    // Getting the page directory of the current page table (the first 10 bits represent 1023 and the next 10 bits 1023 for recursive page lookup)
    //    unsigned long * page_directory = current_page_table -> page_directory;
    unsigned long * page_directory = (unsigned long *) 0xFFFFF000;

    unsigned long temp = 0;

   // FAULT HANDING: CHECKING PAGE DIRECTORY ENTRY 
    // Getting the page table page from the PDE and updating it for recursive page lookup
    //    unsigned long *page_table_page = (unsigned long *)(page_directory[page_dir_index] & PAGE_DIRECTORY_ENTRY_MASK);
    unsigned long *page_table_page = current_page_table->PTE_address(page_dir_index);
    unsigned long page_table_index = (fault_addr >> ADDR_SHIFT_PAGE_TABLE_PAGE) & PAGE_TABLE_PAGE_MASK;
    


   // If the PDE is not valid, then get a frame from the kernel pool to use it as a new page table page
   if ((page_directory[page_dir_index] & USE_BIT)  == 0){
        page_directory[page_dir_index] = (process_mem_pool -> get_frames(PAGE_TABLE_PAGE_FRAME_SIZE))*PAGE_SIZE| WRITE_BIT | USE_BIT;

        for (int i = 0; i<1024; i++) {
            page_table_page[i] = temp | 0x100 ; 
        }
   }
   // FAULT HANDLING: UPDATING PAGE TABLE CORRECTLY
   // If the PTE is not valid, update the PTE by mapping the page table page to the frame address
   // If the PTE is valid, then the page fault is not supposed to occur (hence assert false)
   else if ((page_table_page[page_table_index] & USE_BIT)  == 0) {
        page_table_page[page_table_index] = frame_addr | WRITE_BIT | USE_BIT;
   } else {
        Console::puts("Invalid. Page Fault occured even if the page entry exists\n");
        assert(false);
   }
   return;
}

unsigned long * PageTable::PTE_address(unsigned long addr){
    // Setting the first 10 bits of the entry to represent 1023(last entry of page directory)
    return (unsigned long *) (0xFFC00000 | (addr << ADDR_SHIFT_PAGE_TABLE_PAGE) );
}

void PageTable::register_pool(VMPool * _vm_pool)
{
    
    // Registering the VM pool
    if (this->vm_pool_number < VM_POOL_SIZE) {
        this->vm_pool_register[this->vm_pool_number++] = _vm_pool;
    } else {
        Console::puts("Error while registering pools\n");
        assert(false);
    }
}

void PageTable::free_page(unsigned long _page_no) {
    
    // Getting the page directory index and the page table index
    unsigned long page_dir_index   = _page_no >> ADDR_SHIFT_PAGE_DIRECTORY;
    unsigned long page_table_index   = (_page_no >> ADDR_SHIFT_PAGE_TABLE_PAGE)&PAGE_TABLE_PAGE_MASK;

    // Getting the page table page from the PDE and updating it for recursive page lookup
    unsigned long * page_table_page = this->PTE_address(page_dir_index);

    // Getting the frame number and releasing it
    unsigned long frame  = page_table_page[page_table_index] / PAGE_SIZE;   
    process_mem_pool->release_frames(frame);

    // Updating the page table entry in the page table
    page_table_page[page_table_index] = 0 | WRITE_BIT;

}


