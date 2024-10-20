/**
 * @file aesd-circular-buffer.c
 * @brief Functions and data related to a circular buffer imlementation
 *
 * @author Dan Walkes
 * @date 2020-03-01
 * @copyright Copyright (c) 2020
 *
 */

#ifdef __KERNEL__
#include <linux/string.h>
#else
#include <string.h>
#endif
#include <stdlib.h>
#include "aesd-circular-buffer.h"

/**
 * @param buffer the buffer to search for corresponding offset. Any necessary locking must be performed by caller.
 * @param char_offset the position to search for in the buffer list, describing the zero referenced
 *      character index if all buffer strings were concatenated end to end
 * @param entry_offset_byte_rtn is a pointer specifying a location to store the byte of the returned aesd_buffer_entry
 *      buffptr member corresponding to char_offset.  This value is only set when a matching char_offset is found
 *      in aesd_buffer.
 * @return the struct aesd_buffer_entry structure representing the position described by char_offset, or
 * NULL if this position is not available in the buffer (not enough data is written).
 */
struct aesd_buffer_entry *aesd_circular_buffer_find_entry_offset_for_fpos(struct aesd_circular_buffer *buffer,
            size_t char_offset, size_t *entry_offset_byte_rtn )
{
    // if circular buffer does not exist or is empty, exit
    if(!buffer || !buffer->entry)
    {
        return NULL;
    }
    size_t accumulated_length = 0;
    int8_t entry_id = (buffer->out_offs -1) % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;
    uint8_t offset = 0;

    // Iterating through circular buffer elements
    while(char_offset > accumulated_length)
    {
        entry_id = (entry_id + 1) % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;
        offset = char_offset - accumulated_length;
        accumulated_length += buffer->entry[entry_id].size;

        // The char_offset is higher than the total number of written elements in the circular buffer
        if((entry_id == buffer->in_offs)&&(char_offset > accumulated_length))
        {
            return NULL;
        }
    }
    *entry_offset_byte_rtn = buffer->entry[entry_id].buffptr[offset];
    return &(buffer->entry[entry_id]);
}

/**
* Adds entry @param add_entry to @param buffer in the location specified in buffer->in_offs.
* If the buffer was already full, overwrites the oldest entry and advances buffer->out_offs to the
* new start location.
* Any necessary locking must be handled by the caller
* Any memory referenced in @param add_entry must be allocated by and/or must have a lifetime managed by the caller.
*/
void aesd_circular_buffer_add_entry(struct aesd_circular_buffer *buffer, const struct aesd_buffer_entry *add_entry)
{
    // Check if the current unsigned circular buffer pointer is valid
    if(buffer->in_offs >= AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED)
    {
        return;
    }

    // If buffer full, make room for a new element
    if(buffer->in_offs + 1 == buffer->out_offs)
    {
        buffer->full = true;
        free((char*)(buffer->entry[buffer->in_offs].buffptr));
        buffer->out_offs = (buffer->out_offs + 1) % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;
    }

    // Create a new entry by copying the input entry into the heap.
    char* newEntry = malloc(add_entry->size);
    buffer->entry[buffer->in_offs].buffptr = newEntry;
    buffer->entry[buffer->in_offs].size = add_entry->size;

    buffer->in_offs = (buffer->in_offs + 1) % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;
}

/**
* Initializes the circular buffer described by @param buffer to an empty struct
*/
void aesd_circular_buffer_init(struct aesd_circular_buffer *buffer)
{
    memset(buffer,0,sizeof(struct aesd_circular_buffer));
}
