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
#include <linux/string.h>
#include <linux/types.h>
#include <linux/fs.h> // file_operations
#include "aesdchar.h"
#include "aesd_ioctl.h"

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

// Ajust file offset (f_pos) parameter based on the location specified by
// @param f_pos file offset pointer to be modified
// @param write_cmd the zero referenced command to locate
// @param write_cmd_offset the zero referenced offset into the command
// @return 0 if successful, negative if error occured
int aesd_adjust_file_offset(struct file *filp, loff_t *f_pos, uint32_t write_cmd, uint32_t write_cmd_offset)
{
    PDEBUG("Adjusting file offset with: %u and offset %u", write_cmd, write_cmd_offset);
    int retval = 0;
    int i;
    size_t size_offset = 0;
    struct aesd_dev *dev;

    if(write_cmd > AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED){
        PDEBUG("Write command is outside the range of the circular buffer");
        return -EINVAL;
    }

    dev = filp->private_data;
    if(write_cmd_offset > dev->bufferP.entry[write_cmd].size){
        PDEBUG("Write command offset is outside of the circular buffer entry: %u ", write_cmd_offset);
        return -EINVAL;
    }

    // In this section, we compute the number of bytes used by the write_cmd previous entries
    for (i=0; i<write_cmd; i++)
    {
        size_offset += dev->bufferP.entry[i].size;
    }

    // Set f_pos to the requested position
    *f_pos = size_offset + write_cmd_offset;
    PDEBUG("File offset set to: %lld", *f_pos);
    return retval;
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

// Prepare and call ioctl function command in char* 
int run_ioctl_command(const char *p, struct file *filp, loff_t *f_pos)
{
    PDEBUG("Entering ioctl, working with %s", p);
    int ret;
    char *endptr;
    struct aesd_seekto seekto;

    // Now we are looking for X (command) and Y (offset)
    // Parse the first number (X), look for the comma
    endptr = strchr(p, ',');
    if (!endptr) {
        return -EFAULT;
    }
    // Temporarily terminate the string at the comma, retore after extracting X value
    *endptr = '\0';
    ret = kstrtouint(p, 10, &(seekto.write_cmd));
    *endptr = ',';
    if (ret) {
        PDEBUG("Could not convert X value to an unsigned int");
        return -EFAULT;
    }

    // Parse the second number (Y)
    p = endptr + 1; // Move past the comma
    ret = kstrtouint(p, 10, &(seekto.write_cmd_offset));
    if (ret) {
        PDEBUG("Could not convert Y value to an unsigned int");
        return -EFAULT;
    }

    PDEBUG("Found X = %u and Y = %u", seekto.write_cmd, seekto.write_cmd_offset);

    // Here we bypass the ioctl function, and directly update the f_pos pointer.
    return aesd_adjust_file_offset(filp, f_pos, seekto.write_cmd, seekto.write_cmd_offset);
}

// System call implementation
int aesd_open(struct inode *inode, struct file *filp)
{
    struct aesd_dev *dev; /* device information */
    PDEBUG("open");
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
    struct aesd_dev *dev = filp->private_data;
    PDEBUG("Request %zu bytes read with offset %lld for entry %u",count,*f_pos, dev->bufferP.out_offs);

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
    retval = count > (size_t)retval? retval: count;

    if (copy_to_user(buf, entry->buffptr + entryOffset, retval)) {
        retval = -EFAULT;
        goto out;
    }
    PDEBUG("Read %s, %zd bytes from entry %u of the circular buffer",
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
    struct aesd_dev *dev = filp->private_data;
    const char *prefix = "AESDCHAR_IOCSEEKTO:";
    char *p;
    int newSize;
    PDEBUG("Request write %zu bytes with offset %lld",count,*f_pos);

    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;

    // Allocate more than required so we can reuse the heap memory for partial write case later
    newSize = count+dev->entry.size;
    char* newString = kmalloc(newSize,GFP_KERNEL);

    // Check if received buffer contains an IOCTL request
    if (copy_from_user(newString, buf, count)) {
        retval = -EINVAL;
        goto out;
    }
    if (strstr(newString, prefix) != NULL) {
        PDEBUG("The request is a IOCTL command to set the read pointer");
        // Add a null terminator at the string end as required by kstrtouint
        newString[count] = "\0";

        // Move past the prefix
        p = newString + strlen(prefix);
        if(run_ioctl_command(p, filp, f_pos))
        {
            retval = -EINVAL;
            goto out;
        }
        // Set the proper value to avoid being called again
        retval = count;
        goto out;
    }

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
    // In blocking mode (default), returning 0 or less than count causes retries (partial write)
    return retval;
}

// System call implementation
// The "extended" operations -- only seek
// llseek sets the f_pos value to the requested position
// Requested position defined by offset and whence the reference (begin 0, current 1 or end 2 of file)
loff_t aesd_llseek(struct file *filp, loff_t offset, int whence)
{
    struct aesd_dev *dev = filp->private_data;
    PDEBUG("aesd_llseek called: offset=%lld, whence=%d\n", offset, whence);

    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;

    // Use default llseek method from kernel to update fpos
    filp->f_pos = fixed_size_llseek(filp,offset,whence,dev->size);

    mutex_unlock(&dev->lock);

	return filp->f_pos;
}


// The ioctl() implementation
// Beware of not confusing the ioctl command, cmd, and the SEEKTO command, which is part of arg
// aesd_ioctl can only be called from user space, because copy_from_user will fail if the arg pointer
// is pointing to kernel memory
// Currently, this method is not using any locking mechanism
long aesd_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    long retval = 0;
    struct aesd_seekto seekto;
    PDEBUG("aesd_ioctl called with command : %u, only valid command is : %lu", cmd, AESDCHAR_IOCSEEKTO);
    /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
     */
    if (_IOC_TYPE(cmd) != AESD_IOC_MAGIC) return -ENOTTY;
    if (_IOC_NR(cmd) > AESDCHAR_IOC_MAXNR) return -ENOTTY;

    switch(cmd) {

      case AESDCHAR_IOCSEEKTO:
        if(copy_from_user(&seekto, (const void __user *)arg, sizeof(seekto))) {
            PDEBUG("Failed to copy from user");
            retval = EFAULT;
        } else {
            retval = aesd_adjust_file_offset(filp, &(filp->f_pos), seekto.write_cmd, seekto.write_cmd_offset);
        }
        break;

      default:  /* redundant, as cmd was checked against MAXNR */
        return -ENOTTY;
    }
    return retval;
}


// Table mapping system calls to driver´s functions
// The file_operations describes all the available behaviors of the driver
struct file_operations aesd_fops = {
    .owner =    THIS_MODULE,
	.llseek =   aesd_llseek,
    .read =     aesd_read,
    .write =    aesd_write,
	.unlocked_ioctl = aesd_ioctl,
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
