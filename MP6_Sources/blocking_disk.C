/*
     File        : blocking_disk.c

     Author      : 
     Modified    : 

     Description : 

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "utils.H"
#include "console.H"
#include "blocking_disk.H"
#include "thread.H"
#include "scheduler.H"

extern Scheduler * SYSTEM_SCHEDULER;

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size) 
  : SimpleDisk(_disk_id, _size) {
    // Intialising the head and teal of the disk queue
    head = nullptr;
    tail = nullptr;
    
}



/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/
void BlockingDisk::wait_until_ready() {
     
    if ( !is_ready() ) { 
       // Adding the thread to the end of disk queue when the disk queue is not empty.
        Thread * thread = Thread::CurrentThread();
        if(head == nullptr) {
            head = new disk_queue;
            head->thread = thread;
            head->next = nullptr;
            tail = head;
            // if (!Machine::interrupts_enabled()) { Machine::enable_interrupts(); }
        }else{
            // Adding the thread to the end of disk queue when the disk queue is not empty.
            disk_queue * new_thread = new disk_queue;
            new_thread->thread = thread;
            tail->next = new_thread;
            tail = new_thread;
        }
        // Yeilding the control to the next thread in the ready queue.
        SYSTEM_SCHEDULER->yield();
    }
}

void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf) {
  // -- REPLACE THIS!!!
  SimpleDisk::read(_block_no, _buf);

}


void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf) {
  // -- REPLACE THIS!!!
  SimpleDisk::write(_block_no, _buf);
}

bool BlockingDisk::is_ready() {

  return SimpleDisk::is_ready();
}