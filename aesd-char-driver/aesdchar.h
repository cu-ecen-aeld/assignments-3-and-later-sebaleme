/*
 * aesdchar.h
 *
 *  Created on: Oct 23, 2019
 *      Author: Dan Walkes
 */

#ifndef AESD_CHAR_DRIVER_AESDCHAR_H_
#define AESD_CHAR_DRIVER_AESDCHAR_H_

#define AESD_DEBUG 1  //Remove comment on this line to enable debug

#undef PDEBUG             /* undef it, just in case */
#ifdef AESD_DEBUG
#  ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk( KERN_DEBUG "aesdchar: " fmt, ## args)
#  else
     /* This one for user space */
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#include "aesd-circular-buffer.h"

// Char device has a mutex for exclusive access, a circular buffer to store the data 
// and a buffer entry for handling not null terminated entries.
// See https://www.coursera.org/learn/linux-kernel-programming-yocto-project/lecture/M2Ncq/assignment-8-overview 
struct aesd_dev
{
     struct aesd_buffer_entry entry;       /* buffer for partial data    */
     struct aesd_circular_buffer bufferP;  /* data buffer                */
	struct mutex lock;                    /* mutual exclusion semaphore */
     struct cdev cdev;                     /* Char device structure      */
};


#endif /* AESD_CHAR_DRIVER_AESDCHAR_H_ */
