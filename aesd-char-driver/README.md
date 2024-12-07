# AESD Char Driver

Template source code for the AESD char driver used with assignments 8 and later

Build command:
 cd aesd-char-driver/
 make -C /lib/modules/5.14.0-1051-oem/build M=`pwd` modules
or
 cd aesd-char-driver/
 make -C /lib/modules/6.8.0-48-generic/build M=`pwd` modules

In order to test the module, it can be usefull to check if secureBoot is activated. Can be done with:
 mokutil --sb-state

See this page for documentation list
https://stackoverflow.com/questions/47052640/c-linux-using-pthread-h-in-kernel

See also this docs which is another sum up of the book (explained how the scull drived is called from user space):
https://www-users.cselabs.umn.edu/Fall-2019/csci5103/slides/week14_device_drivers_post.pdf

# Debugging

Meaning of the printf types:
    - %zu  : Stands for size_t, which is an unsigned integer type
    - %lld : Stands for a long long int in signed integer form.
    - %d   : Stands for signed integer type.
More here: https://www.geeksforgeeks.org/format-specifiers-in-c/

Debug messages can be read with dmesg application: 
 dmesg | tail -n 20
