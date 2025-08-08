/*
 File: scheduler.C
 
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

#include "scheduler.H"
#include "thread.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"

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
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/

Scheduler::Scheduler() {

  // Initializing the private members
  head = nullptr;
  tail = nullptr;

}

void Scheduler::yield() {
  // Interrupts are disabled in these schduler functions because they may modify the ready queue which requires mutual exclusion.
  if (Machine::interrupts_enabled()) { Machine::disable_interrupts(); }

  // Checking if the ready queue is empty.
  if(head == nullptr) {
    Console::puts("No thread in empty queue to yield the current one.\n");
    assert(false);
  }

  // If the ready queue is not empty, the first thread in the queue is dispatched.
  ready_queue * temp = head;
  head = head->next;
  // Interrupts are enabled again before leaving the function.
  if (!Machine::interrupts_enabled()) { Machine::enable_interrupts(); }
  Thread::dispatch_to(temp->thread);
}

void Scheduler::resume(Thread * _thread) {
  if (Machine::interrupts_enabled()) { Machine::disable_interrupts(); }

  // Adding the thread to the end of ready queue when the ready queue is empty.
  if(head == nullptr) {
    head = new ready_queue;
    head->thread = _thread;
    head->next = nullptr;
    tail = head;
    if (!Machine::interrupts_enabled()) { Machine::enable_interrupts(); }

    return;
  }

  // Adding the thread to the end of ready queue when the ready queue is not empty.
  ready_queue * new_thread = new ready_queue;
  new_thread->thread = _thread;
  tail->next = new_thread;
  tail = new_thread;
  if (!Machine::interrupts_enabled()) { Machine::enable_interrupts(); }
  return;
}

void Scheduler::add(Thread * _thread) {
  if (Machine::interrupts_enabled()) { Machine::disable_interrupts(); }
  // Adding the thread to the end of ready queue when the ready queue is empty.
  if(head == nullptr) {
    head = new ready_queue;
    head->thread = _thread;
    head->next = nullptr;
    tail = head;
    if (!Machine::interrupts_enabled()) { Machine::enable_interrupts(); }
    return;
  }

  // Adding the thread to the end of ready queue when the ready queue is not empty.
  ready_queue * new_thread = new ready_queue;
  new_thread->thread = _thread;
  tail->next = new_thread;
  tail = new_thread;
  if (!Machine::interrupts_enabled()) { Machine::enable_interrupts(); }
  return;
}

void Scheduler::terminate(Thread * _thread) {
  if (Machine::interrupts_enabled()) { Machine::disable_interrupts(); }
  int id = _thread->ThreadId();

  // If the current running thread is trying to terminate itself.(Thread suicide)
  if(_thread == Thread::CurrentThread()) {
    Console::puts("Thread"); Console::puti(Thread::CurrentThread()->ThreadId()); Console::puts(" suicide.\n");
    if (!Machine::interrupts_enabled()) { Machine::enable_interrupts(); }
    yield();
    return;
  }

  // If the current running thread is trying to terminate another thread in the ready queue.
  //Iterating through the list to remove delete the thread and remove the respective node in the queue
  ready_queue * temp = head;
  if(temp->thread == _thread) {
    head = head->next;
    delete temp;
    if (!Machine::interrupts_enabled()) { Machine::enable_interrupts(); }
    return;
  }
  while(true){
    // If the thread to terminate id not found in the ready queue
    if(temp== tail){
      assert(false);
    }
    if(temp->next->thread == _thread) {
      ready_queue * temp2 = temp->next;
      temp->next = temp->next->next;
      delete temp2;
      delete _thread;
      if (!Machine::interrupts_enabled()) { Machine::enable_interrupts(); }
      return;
    }
  }
  
}


