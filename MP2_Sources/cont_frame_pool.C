/*
 File: ContFramePool.C
 
 Author:
 Date  :
 
 */

/*--------------------------------------------------------------------------*/
/*
 POSSIBLE IMPLEMENTATION
 -----------------------

 The class SimpleFramePool in file "simple_frame_pool.H/C" describes an
 incomplete vanilla implementation of a frame pool that allocates
 *single* frames at a time. Because it does allocate one frame at a time,
 it does not guarantee that a sequence of frames is allocated contiguously.
 This can cause problems.
 
 The class ContFramePool has the ability to allocate either single frames,
 or sequences of contiguous frames. This affects how we manage the
 free frames. In SimpleFramePool it is sufficient to maintain the free
 frames.
 In ContFramePool we need to maintain free *sequences* of frames.
 
 This can be done in many ways, ranging from extensions to bitmaps to
 free-lists of frames etc.
 
 IMPLEMENTATION:
 
 One simple way to manage sequences of free frames is to add a minor
 extension to the bitmap idea of SimpleFramePool: Instead of maintaining
 whether a frame is FREE or ALLOCATED, which requires one bit per frame,
 we maintain whether the frame is FREE, or ALLOCATED, or HEAD-OF-SEQUENCE.
 The meaning of FREE is the same as in SimpleFramePool.
 If a frame is marked as HEAD-OF-SEQUENCE, this means that it is allocated
 and that it is the first such frame in a sequence of frames. Allocated
 frames that are not first in a sequence are marked as ALLOCATED.
 
 NOTE: If we use this scheme to allocate only single frames, then all
 frames are marked as either FREE or HEAD-OF-SEQUENCE.
 
 NOTE: In SimpleFramePool we needed only one bit to store the state of
 each frame. Now we need two bits. In a first implementation you can choose
 to use one char per frame. This will allow you to check for a given status
 without having to do bit manipulations. Once you get this to work,
 revisit the implementation and change it to using two bits. You will get
 an efficiency penalty if you use one char (i.e., 8 bits) per frame when
 two bits do the trick.
 
 DETAILED IMPLEMENTATION:
 
 How can we use the HEAD-OF-SEQUENCE state to implement a contiguous
 allocator? Let's look a the individual functions:
 
 Constructor: Initialize all frames to FREE, except for any frames that you
 need for the management of the frame pool, if any.
 
 get_frames(_n_frames): Traverse the "bitmap" of states and look for a
 sequence of at least _n_frames entries that are FREE. If you find one,
 mark the first one as HEAD-OF-SEQUENCE and the remaining _n_frames-1 as
 ALLOCATED.

 release_frames(_first_frame_no): Check whether the first frame is marked as
 HEAD-OF-SEQUENCE. If not, something went wrong. If it is, mark it as FREE.
 Traverse the subsequent frames until you reach one that is FREE or
 HEAD-OF-SEQUENCE. Until then, mark the frames that you traverse as FREE.
 
 mark_inaccessible(_base_frame_no, _n_frames): This is no different than
 get_frames, without having to search for the free sequence. You tell the
 allocator exactly which frame to mark as HEAD-OF-SEQUENCE and how many
 frames after that to mark as ALLOCATED.
 
 needed_info_frames(_n_frames): This depends on how many bits you need
 to store the state of each frame. If you use a char to represent the state
 of a frame, then you need one info frame for each FRAME_SIZE frames.
 
 A WORD ABOUT RELEASE_FRAMES():
 
 When we releae a frame, we only know its frame number. At the time
 of a frame's release, we don't know necessarily which pool it came
 from. Therefore, the function "release_frame" is static, i.e.,
 not associated with a particular frame pool.
 
 This problem is related to the lack of a so-called "placement delete" in
 C++. For a discussion of this see Stroustrup's FAQ:
 http://www.stroustrup.com/bs_faq2.html#placement-delete
 
 */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "cont_frame_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"

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
/* METHODS FOR CLASS   C o n t F r a m e P o o l */
/*--------------------------------------------------------------------------*/

ContFramePool* ContFramePool::head = nullptr;

ContFramePool::FrameState ContFramePool::get_state(unsigned long _frame_no){

    // Using two bits to store status of one frame. Thus a character is used to store status of 4 frames.
    // Case 1: Frame is free => maska = 0, maskb = 0
    // Case 2: Frame is used => maska = 1, maskb = 0
    // Case 3: Frame is HoS => maska = 1, maskb = 1

    unsigned int bitmap_index = _frame_no / 4;
    unsigned char maska = 0x1 << ((_frame_no*2) % 8);
    unsigned char maskb = 0x1 << ((_frame_no*2) % 8) + 1;
    if(bitmap[bitmap_index] & maska){
        if(bitmap[bitmap_index] & maskb){
            return FrameState::HoS;
        }
        else{
            return FrameState::Used;
        }
    }
    else{
        return FrameState::Free;
        
    }
}

void ContFramePool::set_state(unsigned long _frame_no, FrameState _state){
    
    // Using two bits to store status of one frame. Thus a character is used to store status of 4 frames.
    // Case 1: Frame is free => maska = 0, maskb = 0
    // Case 2: Frame is used => maska = 1, maskb = 0
    // Case 3: Frame is HoS => maska = 1, maskb = 1


    unsigned int bitmap_index = _frame_no / 4;
    unsigned char maska = 0x1 << ((_frame_no*2) % 8);
    unsigned char maskb = 0x1 << (((_frame_no*2) % 8) + 1);
    if(_state == FrameState::Free){
        
        bitmap[bitmap_index] &= ~maska;
        bitmap[bitmap_index] &= ~maskb;
    }
    else if(_state == FrameState::Used){
        bitmap[bitmap_index] |= maska;
        bitmap[bitmap_index] &= ~maskb;
    }
    else if(_state == FrameState::HoS){
        bitmap[bitmap_index] |= maska;
        bitmap[bitmap_index] |= maskb;
    }
    else{
        Console::puts("Error: Frame is in an invalid state\n");
        assert(false);
    }
    

}



ContFramePool::ContFramePool(unsigned long _base_frame_no,
                             unsigned long _n_frames,
                             unsigned long _info_frame_no)
{

    // Updating the linked list of frame pools
    // Console::puts("Info: Intializing Frame Pool\n");
    ContFramePool* it = head;
    ContFramePool* prev = nullptr;
    while(it != nullptr){
        unsigned long base = it->base_frame_no;
        unsigned long end = base + it->nframes - 1;
        // if((base>=_base_frame_no && base<_base_frame_no+_n_frames) || (end>=_base_frame_no && end<_base_frame_no+_n_frames)){
        //     Console::puts("Error: Given Frame pool overlaps with already existing frame pool\n");
        //     assert(false);
        // }
        prev = it;
        it = it->next;
    }
    if (prev == nullptr){
        head = this;
    }
    else{
        prev->next = this;
    }
    
    // Bitmap must fit in a single frame!
    if(_n_frames > FRAME_SIZE * 4){
        Console::puts("Error: Frame pool is too large in fit metadata in one frame\n");
        assert(false);
    }
    
    
    // Initialize the class variables
    this -> base_frame_no = _base_frame_no;
    this -> nframes = _n_frames;
    this -> nFreeFrames = _n_frames;
    this -> info_frame_no = _info_frame_no;
    
    // If _info_frame_no is zero then we keep management info in the first
    //frame, else we use the provided frame to keep management info
    if(info_frame_no == 0) {
        bitmap = (unsigned char *) (this -> base_frame_no * FRAME_SIZE);

    } else {
        bitmap = (unsigned char *) (this -> info_frame_no * FRAME_SIZE);
    }

    // Everything ok. Proceed to mark all frame as free.
    for(int i = 0; i < _n_frames; i++) {
        set_state(i, FrameState::Free);
    }


    // If _info_frame_no is zero then we keep management info in the first. Set the status of info frame as HoS
    if(info_frame_no == 0){
        set_state(0, FrameState::HoS);
        nFreeFrames--;
    }
    else{
        set_state(this->info_frame_no - this->base_frame_no, FrameState::HoS);
        nFreeFrames--;
    }
    
    // Console::puts("Info: Frame Pool initialized\n");
    
}

unsigned long ContFramePool::get_frames(unsigned int _n_frames)
{
    // Iterating through the bitmap to find the first sequence of _n_frames free frames
    // Console::puts("Info: Allocating Frames\n");
    unsigned long i = 0;
    while(i<nframes){
        unsigned long init = i;
        while(get_state(i) == FrameState::Free && (i< nframes)){
            i++;
            if(i-init==_n_frames){
                set_state(init, FrameState::HoS);
                for(unsigned long j = init+1; j < i; j++){
                    set_state(j, FrameState::Used);
                }
                nFreeFrames -= _n_frames;
                // Console::puts("Info: Frames allocated\n");
                return base_frame_no + init;
            }
        }
        i++;
    }


    // Console::puts("Error: Not enough contiguous frames to allocate\n");
    return 0;
}



void ContFramePool::release_frames(unsigned long _first_frame_no)
{
    // Releasing the frames starting from _first_frame_no
    // Console::puts("Info: Releasing Frames\n");
    ContFramePool* it = head;
    while(it!=nullptr){
        // Checking if the given frame is in the current frame pool
        if(_first_frame_no >= it->base_frame_no && _first_frame_no < it->base_frame_no + it->nframes){
            unsigned long base = it->base_frame_no;
            if(it->get_state(_first_frame_no - base) != FrameState::HoS){
                Console::puts("Error: Frame is not the head of a sequence\n");
                assert(false);
                return;
            }else{
                // Marking the frames as free
                it->set_state(_first_frame_no - base, FrameState::Free);
                it->nFreeFrames++;
                unsigned long i = 1;
                while(it->get_state(_first_frame_no - base + i) == FrameState::Used){
                    it->set_state(_first_frame_no - base + i, FrameState::Free);
                    it->nFreeFrames++;
                    i++;
                }
                return;
            }
        }
        it = it->next;
    }

    Console::puts("Error: Frame not found\n");
    assert(false);
    return;
    
}

void ContFramePool::mark_inaccessible(unsigned long _base_frame_no,
                                      unsigned long _n_frames)
{
    // Console::puts("Info: Marking the frames as inaccesible\n");
    set_state(0, FrameState::HoS);
    for(unsigned long i = 1; i < _n_frames; i++){
        set_state(i, FrameState::Used);
    }
    // Console::puts("Info: Marked the frames as inaccesible\n");
}

unsigned long ContFramePool::needed_info_frames(unsigned long _n_frames)
{
    // Each byte can store 4 frames. Thus, we need _n_frames/(4*FRAME_SIZE) info frames
    // Console::puts("Info: Calculating the no.of info frames\n");
    if(_n_frames % (FRAME_SIZE*4) == 0)
        return _n_frames / (FRAME_SIZE*4);
    
    return (_n_frames / FRAME_SIZE*4) + 1;
    // Console::puts("Info: Returned no.of info frames\n");

}
