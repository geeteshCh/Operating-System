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
#include "mirror_disk.H"
#include "thread.H"
#include "scheduler.H"
#include "simple_disk.H"

extern Scheduler * SYSTEM_SCHEDULER;

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/



void issue_operation_mirror(DISK_OPERATION _op, unsigned long _block_no,DISK_ID disk_id)
{
  // Defined a s custon issue operation to handle the read and write operations for the master and slave disk separately.
  Machine::outportb(0x1F1, 0x00);
  Machine::outportb(0x1F2, 0x01); 
  Machine::outportb(0x1F3, (unsigned char)_block_no);
  Machine::outportb(0x1F4, (unsigned char)(_block_no >> 8));
  Machine::outportb(0x1F5, (unsigned char)(_block_no >> 16));
  unsigned int disk_no = disk_id == DISK_ID::MASTER ? 0 : 1;
  Machine::outportb(0x1F6, ((unsigned char)(_block_no >> 24)&0x0F) | 0xE0 | (disk_no << 4));
  Machine::outportb(0x1F7, (_op == DISK_OPERATION::READ) ? 0x20 : 0x30);

}


/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/
MirrorDisk::MirrorDisk(DISK_ID _disk_id, unsigned int _size) 
  : BlockingDisk(_disk_id, _size) {
    return;
    
}
void MirrorDisk::wait_until_ready() {
     
    if ( !is_ready() ) { 
        // Adding the thread to the end of disk queue when the disk queue is empty.

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
        SYSTEM_SCHEDULER->yield();
    }
}

void MirrorDisk::read(unsigned long _block_no, unsigned char * _buf) {
  //Handling issue operations for the master and slave disk separately.
  issue_operation_mirror(DISK_OPERATION::READ, _block_no, DISK_ID:: MASTER);
  issue_operation_mirror(DISK_OPERATION::READ, _block_no, DISK_ID::DEPENDENT);

 // Copied the read function from the simple disk class to read the data from the master and slave disk.
  wait_until_ready();

  /* read data from port */
  int i;
  unsigned short tmpw;
  for (i = 0; i < 256; i++) {
    tmpw = Machine::inportw(0x1F0);
    _buf[i*2]   = (unsigned char)tmpw;
    _buf[i*2+1] = (unsigned char)(tmpw >> 8);
  }

}


void MirrorDisk::write(unsigned long _block_no, unsigned char * _buf) {
  // -- REPLACE THIS!!!
  MASTER_DISK->write(_block_no, _buf);
  SLAVE_DISK->write(_block_no, _buf);
}

bool MirrorDisk::is_ready() {

  return SimpleDisk::is_ready();
}