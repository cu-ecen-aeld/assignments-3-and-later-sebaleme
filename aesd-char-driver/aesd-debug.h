/*
 * aesd-debug.h
 *
 *  Created on: Nov 24th, 2024
 *      Author: Sebastien Lemetter
 */

#ifndef AESD_CHAR_DRIVER_AESD_DEBUG_H_
#define AESD_CHAR_DRIVER_AESD_DEBUG_H_

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

#endif /* AESD_CHAR_DRIVER_AESD_DEBUG_H_ */
