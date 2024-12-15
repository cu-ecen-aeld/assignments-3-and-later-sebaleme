/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * @date 2019-10-22
 * @copyright Copyright (c) 2019
 *
 */

#include <linux/cdev.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/slab.h>         // kmalloc()
#include <linux/types.h>
#include <linux/fs.h> // file_operations
#include "aesdchar.h"

int aesd_major =   0; // use dynamic major
int aesd_minor =   0;

MODULE_AUTHOR("Sebastien Lemetter");
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device;

void clean_aesd(void)
{
    uint8_t index;
    struct aesd_buffer_entry *entry;
    PDEBUG("Erase device");
    AESD_CIRCULAR_BUFFER_FOREACH(entry,&aesd_device.bufferP,index) {
        kfree(entry->buffptr);
    }
}

// Return the position of the first EOL char in char array
int checkEOLChar(const char* buff, const int size)
{
    int i;
    for(i=0; i<size; i++)
    {
        if(buff[i]=='\n')
        {
            return i;
        }
    }
    return -1;
}

// Update circular buffer pointer according to fpos value
void updatePointers(struct aesd_dev *dev, loff_t newpos)
{

}

// Check if single entry should be written into circular buffer
void write_entry_into_buffer(struct aesd_dev *dev)
{
        // Now check if current write has EOL character
        int EOLPos = checkEOLChar(dev->entry.buffptr, dev->entry.size);
        PDEBUG("Found EOL char at pos %d ", EOLPos);
        if(EOLPos == dev->entry.size-1)
        {
            // TODO: Need to remember the potentially removed item, remove when add_entry returns circular entry
            size_t sizeToRemove = dev->bufferP.entry[dev->bufferP.in_offs].size;
            const char* entryToRemove = aesd_circular_buffer_add_entry(&(dev->bufferP), &(dev->entry));
            if(entryToRemove)
            {
                PDEBUG("Removing entry: %s", entryToRemove);
                dev->size -= sizeToRemove;
                kfree(entryToRemove);
            }
            // Update circular buffer size
            dev->size += dev->entry.size;

            // Remove the content from the entry buffer since moved to circular buffer
            dev->entry.buffptr = NULL;
            dev->entry.size = 0;
        }
        else if(EOLPos < 0)
        {
            // No EOL char, meaning we only store in entry buffer
            PDEBUG("Written in entry buffer");
        }
        else
        {
            // EOL char found inside the char array
            PDEBUG("Written in entry buffer");
        }
}

// System call implementation
int aesd_open(struct inode *inode, struct file *filp)
{
    PDEBUG("open");

    struct aesd_dev *dev; /* device information */
    dev = container_of(inode->i_cdev, struct aesd_dev, cdev);
    filp->private_data = dev; /* for other methods */

    // Note: for usual files, open in write only mode (echo "" >) leads to erase the previous content
    // and open in appending mode (open "" >>) just write at the end of the file.
    // In aesdchar, this is not the case, we always append the previous content in another buffer entry,
    // and once the complete circular buffer size has been written, then we overwrite the oldest content,
    // regardless of the mode used to open the device (> or >>).
    
    // Return the content (or partial content) related to the most recent 10 write commands, in the order
    // they were received, on any read attempt. So if the buffer is not full, indice 0 will allways be the
    // first to be received, so should be the first to be returned.
    if (!dev->bufferP.full)
    {
        dev->bufferP.out_offs = 0;
    }
    return 0;
}

int aesd_release(struct inode *inode, struct file *filp)
{
    PDEBUG("release");
    /**
     * TODO: handle release
     */
    return 0;
}

// System call implementation
// Return the content (or partial content) related to the most recent 10 write commands, in the order
// they were received, on any read attempt.
// f_pos is used here to handle partial read
ssize_t aesd_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = 0;
    size_t entryOffset;
    struct aesd_buffer_entry* entry;
    PDEBUG("Request %zu bytes read with offset %lld",count,*f_pos);
    struct aesd_dev *dev = filp->private_data;

    if (mutex_lock_interruptible(&dev->lock)) {
        return -ERESTARTSYS;
    }

    entry = aesd_circular_buffer_find_entry_offset_for_fpos(&(dev->bufferP), *f_pos, &entryOffset);
    if(!entry)
    {
        PDEBUG("No entrie was written yet, so do nothing");
        goto out;
    }

    // How many bytes should we copy. Either the entry content minus offset, or just what was requested
    retval = entry->size - entryOffset;
    retval = count ? count < retval : retval;

    if (copy_to_user(buf, entry->buffptr + entryOffset, retval)) {
        retval = -EFAULT;
        goto out;
    }
    PDEBUG("Read %s, %zu bytes from entry %u of the circular buffer",
                entry->buffptr + entryOffset,
                retval, 
                dev->bufferP.out_offs
    );
    *f_pos += retval;

    out:
        PDEBUG("Returns %zd bytes with new offset %lld, new read pointer set to %u",retval,*f_pos, dev->bufferP.out_offs);
        mutex_unlock(&dev->lock);
        return retval;
}

// System call implementation
// Write position (f_pos) and write file offsets can be ignored on this assignment
ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = -ENOMEM;
    PDEBUG("Request write %zu bytes with offset %lld",count,*f_pos);
    struct aesd_dev *dev = filp->private_data; 

    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;
    
    // We have 4 uses cases here.
    // Cases with empty entry buffer:
    // - New write terminated by an EOL charater: write in entry buffer, copy in circular buffer and free entry buffer
    // - New write without EOL character: write in entry buffer
    // Cases with filled entry buffer:
    // - New write terminated by an EOL charater: kmalloc a new entry with the concatenated char arrays and free previous
    //   instance, copy in circular buffer and free entry buffer
    // - New write without EOL character: kmalloc a new entry with the concatenated char arrays and free previous instance

    // Partial write ongoing
    if(dev->entry.buffptr)
    {
        PDEBUG("Proceeding a partial write");
        int newSize = count+dev->entry.size;
        char* newString = kmalloc(newSize,GFP_KERNEL);
        memcpy(newString, dev->entry.buffptr, dev->entry.size);
        // Here we do pointer arithmetic to concatenate previous and new content
        if (copy_from_user(newString + dev->entry.size, buf, count)) {
            retval = -EFAULT;
            goto out;
        }
        // Replace the entry buffer with the new content
        kfree(dev->entry.buffptr);
        dev->entry.buffptr = newString;
        dev->entry.size = newSize;

        write_entry_into_buffer(dev);
    }
    else
    {
        PDEBUG("Starting a new write session");
        // Since entry buffer was not used, we can directly allocate it
        dev->entry.buffptr = kmalloc(count,GFP_KERNEL);
        dev->entry.size = count;
        if (copy_from_user(dev->entry.buffptr, buf, dev->entry.size)) {
            retval = -EFAULT;
            goto out;
        }
        write_entry_into_buffer(dev);
    }

    *f_pos += count;
    retval = count;
  out:
    mutex_unlock(&dev->lock);
    PDEBUG("%zd bytes were written, new total size is %lu",retval, dev->size);
    return retval;
}

/*
 * The "extended" operations -- only seek
 */
loff_t aesd_llseek(struct file *filp, loff_t offset, int whence)
{
    PDEBUG("aesd_llseek called: offset=%lld, whence=%d\n", offset, whence);
    struct aesd_dev *dev = filp->private_data; 

    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;

    // Use default llseek method from kernel to update fpos
    filp->f_pos = fixed_size_llseek(filp,offset,whence,dev->size);

    // Update read pointer according to new file offset
    updatePointers(dev, filp->f_pos);

    mutex_unlock(&dev->lock);

	return filp->f_pos;
}


// Table mapping system calls to driver´s functions
// The file_operations describes all the available behaviors of the driver
struct file_operations aesd_fops = {
    .owner =    THIS_MODULE,
	.llseek =   aesd_llseek,
    .read =     aesd_read,
    .write =    aesd_write,
    .open =     aesd_open,
    .release =  aesd_release,
};

static int aesd_setup_cdev(struct aesd_dev *dev)
{
    int err, devno = MKDEV(aesd_major, aesd_minor);

    cdev_init(&dev->cdev, &aesd_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &aesd_fops;
    err = cdev_add (&dev->cdev, devno, 1);
    if (err) {
        printk(KERN_ERR "Error %d adding aesd cdev", err);
    }
    return err;
}



int aesd_init_module(void)
{
    dev_t dev = 0;
    int result;
    result = alloc_chrdev_region(&dev, aesd_minor, 1,
            "aesdchar");
    aesd_major = MAJOR(dev);
    if (result < 0) {
        printk(KERN_WARNING "Can't get major %d\n", aesd_major);
        return result;
    }
    // Here, the device, the circular buffer and the circular buffer entry are initialized
    memset(&aesd_device,0,sizeof(struct aesd_dev));
    aesd_circular_buffer_init(&aesd_device.bufferP);
    aesd_device.entry.buffptr = NULL;
    result = aesd_setup_cdev(&aesd_device);
	mutex_init(&aesd_device.lock);

    if( result ) {
        unregister_chrdev_region(dev, 1);
    }
    return result;
}

void aesd_cleanup_module(void)
{
    dev_t devno = MKDEV(aesd_major, aesd_minor);

    cdev_del(&aesd_device.cdev);
    clean_aesd();
    unregister_chrdev_region(devno, 1);
}



module_init(aesd_init_module);
module_exit(aesd_cleanup_module);
