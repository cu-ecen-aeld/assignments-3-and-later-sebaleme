# AESD Char Driver

Template source code for the AESD char driver used with assignments 8 and later

Build command:
 cd aesd-char-driver/
 make -C /lib/modules/5.14.0-1051-oem/build M=`pwd` modules
or
 cd aesd-char-driver/
 make -C /lib/modules/6.8.0-48-generic/build M=`pwd` modules

See this page for documentation list
https://stackoverflow.com/questions/47052640/c-linux-using-pthread-h-in-kernel
