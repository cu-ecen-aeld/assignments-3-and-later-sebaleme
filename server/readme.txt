### Runtime profiling

## Configuration

The sockettest.sh was modified as follow:
ts=$(date +%s%N)
// Test execution
duration=$SECONDS
echo "Tests complete with success in $((($(date +%s%N) - $ts)/1000000))ms!"


## Results of sockettest, single thread

The single threaded application yields the following results:

Here selected outputs from the /var/log/syslog:
Oct  6 15:17:25 FE-C-013E3 aesdsocket: Closed connection from 127.0.0.1:61674 after 0.000230 seconds
Oct  6 15:17:27 FE-C-013E3 aesdsocket: Closed connection from 127.0.0.1:62186 after 0.000136 seconds
Oct  6 15:17:28 FE-C-013E3 aesdsocket: Closed connection from 127.0.0.1:64234 after 0.000133 seconds
Oct  6 15:17:29 FE-C-013E3 aesdsocket: Closed connection from 127.0.0.1:64746 after 0.000231 seconds
Oct  6 15:17:30 FE-C-013E3 aesdsocket: Closed connection from 127.0.0.1:2795 after 0.000424 seconds
This makes a total of 1.154ms C-code execution, the rest being the bash script and data transmission

The complete test duration:
Tests complete with success in 5145ms!


## Results of sockettest, multi thread

Here selected outputs from the /var/log/syslog:
Oct  8 18:15:55 FE-C-013E3 aesdsocket: Entering server socket program
Oct  8 18:15:55 FE-C-013E3 aesdsocket: Socket created and binded, with file descriptor 4
Oct  8 18:15:55 FE-C-013E3 aesdsocket: Listening to connections on 4
Oct  8 18:15:55 FE-C-013E3 aesdsocket: timestamp thread started, now 1 ongoing
Oct  8 18:16:00 FE-C-013E3 aesdsocket: New thread started, now 2 ongoing
Oct  8 18:16:00 FE-C-013E3 aesdsocket: Accepted connection from 127.0.0.1:56982
Oct  8 18:16:00 FE-C-013E3 aesdsocket: Received 8 bytes, wrote 8 bytes into target file
Oct  8 18:16:00 FE-C-013E3 aesdsocket: Read 8 bytes in local file, sent 8 bytes as acknowledgement, ||abcdefg#012||
Oct  8 18:16:01 FE-C-013E3 aesdsocket: Thread 5 finished, received a total of 8 data from the client after 0.000315 seconds
Oct  8 18:16:01 FE-C-013E3 aesdsocket: New thread started, now 3 ongoing
Oct  8 18:16:01 FE-C-013E3 aesdsocket: Accepted connection from 127.0.0.1:57494
Oct  8 18:16:01 FE-C-013E3 aesdsocket: Received 10 bytes, wrote 10 bytes into target file
Oct  8 18:16:01 FE-C-013E3 aesdsocket: Read 18 bytes in local file, sent 18 bytes as acknowledgement, ||abcdefg#012hijklmnop#012||
Oct  8 18:16:01 FE-C-013E3 aesdsocket: Thread 5 closed
Oct  8 18:16:02 FE-C-013E3 aesdsocket: Thread 6 finished, received a total of 10 data from the client after 0.000276 seconds
Oct  8 18:16:02 FE-C-013E3 aesdsocket: Thread 6 closed
Oct  8 18:16:02 FE-C-013E3 aesdsocket: New thread started, now 2 ongoing
Oct  8 18:16:02 FE-C-013E3 aesdsocket: Accepted connection from 127.0.0.1:60566
Oct  8 18:16:02 FE-C-013E3 aesdsocket: Received 11 bytes, wrote 11 bytes into target file
Oct  8 18:16:02 FE-C-013E3 aesdsocket: Read 29 bytes in local file, sent 29 bytes as acknowledgement, ||abcdefg#012hijklmnop#0121234567890#012||
Oct  8 18:16:03 FE-C-013E3 aesdsocket: Thread 5 finished, received a total of 11 data from the client after 0.000285 seconds
Oct  8 18:16:04 FE-C-013E3 aesdsocket: New thread started, now 3 ongoing
Oct  8 18:16:04 FE-C-013E3 aesdsocket: Accepted connection from 127.0.0.1:36994
Oct  8 18:16:04 FE-C-013E3 aesdsocket: Received 11 bytes, wrote 11 bytes into target file
Oct  8 18:16:04 FE-C-013E3 aesdsocket: Read 40 bytes in local file, sent 40 bytes as acknowledgement, ||abcdefg#012hijklmnop#0121234567890#0129876543210#012||
Oct  8 18:16:04 FE-C-013E3 aesdsocket: Thread 5 closed
Oct  8 18:16:05 FE-C-013E3 aesdsocket: Thread 6 finished, received a total of 11 data from the client after 0.000283 seconds
Oct  8 18:16:05 FE-C-013E3 aesdsocket: Thread 6 closed
Oct  8 18:16:05 FE-C-013E3 aesdsocket: New thread started, now 2 ongoing
Oct  8 18:16:05 FE-C-013E3 aesdsocket: Accepted connection from 127.0.0.1:39042
Oct  8 18:16:05 FE-C-013E3 aesdsocket: Received 55 bytes, wrote 55 bytes into target file
Oct  8 18:16:05 FE-C-013E3 aesdsocket: Read 95 bytes in local file, sent 95 bytes as acknowledgement, ||abcdefg#012hijklmnop#0121234567890#0129876543210#012If you want to shine like a sun, first burn like a sun#012||
Oct  8 18:16:05 FE-C-013E3 aesdsocket: New thread started, now 3 ongoing
Oct  8 18:16:05 FE-C-013E3 aesdsocket: Accepted connection from 127.0.0.1:42114
Oct  8 18:16:05 FE-C-013E3 aesdsocket: Received 86 bytes, wrote 86 bytes into target file
Oct  8 18:16:05 FE-C-013E3 aesdsocket: Read 181 bytes in local file, sent 181 bytes as acknowledgement, ||abcdefg#012hijklmnop#0121234567890#0129876543210#012If you want to shine like a sun, first burn like a sun#012Never stop fighting until you arrive at your destined place - that is, the unique you#012||
Oct  8 18:16:05 FE-C-013E3 aesdsocket: New thread started, now 4 ongoing
Oct  8 18:16:05 FE-C-013E3 aesdsocket: Accepted connection from 127.0.0.1:44162
Oct  8 18:16:05 FE-C-013E3 aesdsocket: Received 92 bytes, wrote 92 bytes into target file
Oct  8 18:16:05 FE-C-013E3 aesdsocket: Read 273 bytes in local file, sent 273 bytes as acknowledgement, ||abcdefg#012hijklmnop#0121234567890#0129876543210#012If you want to shine like a sun, first burn like a sun#012Never stop fighting until you arrive at your destined place - that is, the unique you#012One best book is equal to a hundred good friends, but one good friend is equal to a library#012||
Oct  8 18:16:05 FE-C-013E3 aesdsocket: Timestamp thread: wrote 28 bytes into target file
Oct  8 18:16:06 FE-C-013E3 aesdsocket: Thread 5 finished, received a total of 55 data from the client after 0.001217 seconds
Oct  8 18:16:06 FE-C-013E3 aesdsocket: New thread started, now 5 ongoing
Oct  8 18:16:06 FE-C-013E3 aesdsocket: Accepted connection from 127.0.0.1:44674
Oct  8 18:16:06 FE-C-013E3 aesdsocket: Received 55 bytes, wrote 55 bytes into target file
Oct  8 18:16:06 FE-C-013E3 aesdsocket: Read 356 bytes in local file, sent 356 bytes as acknowledgement, ||abcdefg#012hijklmnop#0121234567890#0129876543210#012If you want to shine like a sun, first burn like a sun#012Never stop fighting until you arrive at your destined place - that is, the unique you#012One best book is equal to a hundred good friends, but one good friend is equal to a library#012timestamp:20241008 18:16:05#012If you want to shine like a sun, first burn like a sun#012||
Oct  8 18:16:06 FE-C-013E3 aesdsocket: Thread 6 finished, received a total of 86 data from the client after 0.001493 seconds
Oct  8 18:16:06 FE-C-013E3 aesdsocket: Thread 5 closed
Oct  8 18:16:06 FE-C-013E3 aesdsocket: Thread 7 finished, received a total of 92 data from the client after 0.001287 seconds
Oct  8 18:16:07 FE-C-013E3 aesdsocket: Thread 8 finished, received a total of 55 data from the client after 0.000613 seconds
Oct  8 18:16:07 FE-C-013E3 aesdsocket: Thread 8 closed
Oct  8 18:16:07 FE-C-013E3 aesdsocket: Thread 7 closed
Oct  8 18:16:07 FE-C-013E3 aesdsocket: Thread 6 closed
Oct  8 18:16:07 FE-C-013E3 aesdsocket: New thread started, now 2 ongoing
Oct  8 18:16:07 FE-C-013E3 aesdsocket: Accepted connection from 127.0.0.1:47234
Oct  8 18:16:07 FE-C-013E3 aesdsocket: Received 86 bytes, wrote 86 bytes into target file
Oct  8 18:16:07 FE-C-013E3 aesdsocket: Read 442 bytes in local file, sent 442 bytes as acknowledgement, ||abcdefg#012hijklmnop#0121234567890#0129876543210#012If you want to shine like a sun, first burn like a sun#012Never stop fighting until you arrive at your destined place - that is, the unique you#012One best book is equal to a hundred good friends, but one good friend is equal to a library#012timestamp:20241008 18:16:05#012If you want to shine like a sun, first burn like a sun#012Never stop fighting until you arrive at your destined place - that is, the unique you#012||
Oct  8 18:16:07 FE-C-013E3 aesdsocket: New thread started, now 3 ongoing
Oct  8 18:16:07 FE-C-013E3 aesdsocket: Accepted connection from 127.0.0.1:48258
Oct  8 18:16:07 FE-C-013E3 aesdsocket: Received 92 bytes, wrote 92 bytes into target file
Oct  8 18:16:07 FE-C-013E3 aesdsocket: Read 534 bytes in local file, sent 534 bytes as acknowledgement, ||abcdefg#012hijklmnop#0121234567890#0129876543210#012If you want to shine like a sun, first burn like a sun#012Never stop fighting until you arrive at your destined place - that is, the unique you#012One best book is equal to a hundred good friends, but one good friend is equal to a library#012timestamp:20241008 18:16:05#012If you want to shine like a sun, first burn like a sun#012Never stop fighting until you arrive at your destined place - that is, the unique you#012One best book is equal to a hundred good friends, but one good friend is equal to a library#012||
Oct  8 18:16:07 FE-C-013E3 aesdsocket: New thread started, now 4 ongoing
Oct  8 18:16:07 FE-C-013E3 aesdsocket: Accepted connection from 127.0.0.1:48770
Oct  8 18:16:07 FE-C-013E3 aesdsocket: Received 55 bytes, wrote 55 bytes into target file
Oct  8 18:16:07 FE-C-013E3 aesdsocket: Read 589 bytes in local file, sent 589 bytes as acknowledgement, ||abcdefg#012hijklmnop#0121234567890#0129876543210#012If you want to shine like a sun, first burn like a sun#012Never stop fighting until you arrive at your destined place - that is, the unique you#012One best book is equal to a hundred good friends, but one good friend is equal to a library#012timestamp:20241008 18:16:05#012If you want to shine like a sun, first burn like a sun#012Never stop fighting until you arrive at your destined place - that is, the unique you#012One best book is equal to a hundred good friends, but one good friend is equal to a library#012If you want to shine like a sun, first burn like a sun#012||
Oct  8 18:16:08 FE-C-013E3 aesdsocket: Thread 5 finished, received a total of 86 data from the client after 0.001171 seconds
Oct  8 18:16:08 FE-C-013E3 aesdsocket: New thread started, now 5 ongoing
Oct  8 18:16:08 FE-C-013E3 aesdsocket: Accepted connection from 127.0.0.1:50306
Oct  8 18:16:08 FE-C-013E3 aesdsocket: Received 86 bytes, wrote 86 bytes into target file
Oct  8 18:16:08 FE-C-013E3 aesdsocket: Read 675 bytes in local file, sent 675 bytes as acknowledgement, ||abcdefg#012hijklmnop#0121234567890#0129876543210#012If you want to shine like a sun, first burn like a sun#012Never stop fighting until you arrive at your destined place - that is, the unique you#012One best book is equal to a hundred good friends, but one good friend is equal to a library#012timestamp:20241008 18:16:05#012If you want to shine like a sun, first burn like a sun#012Never stop fighting until you arrive at your destined place - that is, the unique you#012One best book is equal to a hundred good friends, but one good friend is equal to a library#012If you want to shine like a sun, first burn like a sun#012Never stop fighting until you arrive at your destined place - that is, the unique you#012||
Oct  8 18:16:08 FE-C-013E3 aesdsocket: Thread 6 finished, received a total of 92 data from the client after 0.001405 seconds
Oct  8 18:16:08 FE-C-013E3 aesdsocket: Thread 5 closed
Oct  8 18:16:08 FE-C-013E3 aesdsocket: Thread 7 finished, received a total of 55 data from the client after 0.001150 seconds
Oct  8 18:16:09 FE-C-013E3 aesdsocket: Thread 8 finished, received a total of 86 data from the client after 0.000637 seconds
Oct  8 18:16:09 FE-C-013E3 aesdsocket: Thread 8 closed
Oct  8 18:16:09 FE-C-013E3 aesdsocket: Thread 7 closed
Oct  8 18:16:09 FE-C-013E3 aesdsocket: Thread 6 closed
Oct  8 18:16:09 FE-C-013E3 aesdsocket: New thread started, now 2 ongoing
Oct  8 18:16:09 FE-C-013E3 aesdsocket: Accepted connection from 127.0.0.1:52354
Oct  8 18:16:09 FE-C-013E3 aesdsocket: Received 92 bytes, wrote 92 bytes into target file
Oct  8 18:16:09 FE-C-013E3 aesdsocket: Read 767 bytes in local file, sent 767 bytes as acknowledgement, ||abcdefg#012hijklmnop#0121234567890#0129876543210#012If you want to shine like a sun, first burn like a sun#012Never stop fighting until you arrive at your destined place - that is, the unique you#012One best book is equal to a hundred good friends, but one good friend is equal to a library#012timestamp:20241008 18:16:05#012If you want to shine like a sun, first burn like a sun#012Never stop fighting until you arrive at your destined place - that is, the unique you#012One best book is equal to a hundred good friends, but one good friend is equal to a library#012If you want to shine like a sun, first burn like a sun#012Never stop fighting until you arrive at your destined place - that is, the unique you#012One best book is equal to a hundred good friends, but one good friend is equal to a library#012||
Oct  8 18:16:10 FE-C-013E3 aesdsocket: Thread 5 finished, received a total of 92 data from the client after 0.000295 seconds
Oct  8 18:16:11 FE-C-013E3 aesdsocket: New thread started, now 3 ongoing
Oct  8 18:16:11 FE-C-013E3 aesdsocket: Accepted connection from 127.0.0.1:55938
Oct  8 18:16:11 FE-C-013E3 aesdsocket: Received 23 bytes, wrote 23 bytes into target file
Oct  8 18:16:11 FE-C-013E3 aesdsocket: Read 790 bytes in local file, sent 790 bytes as acknowledgement, ||abcdefg#012hijklmnop#0121234567890#0129876543210#012If you want to shine like a sun, first burn like a sun#012Never stop fighting until you arrive at your destined place - that is, the unique you#012One best book is equal to a hundred good friends, but one good friend is equal to a library#012timestamp:20241008 18:16:05#012If you want to shine like a sun, first burn like a sun#012Never stop fighting until you arrive at your destined place - that is, the unique you#012One best book is equal to a hundred good friends, but one good friend is equal to a library#012If you want to shine like a sun, first burn like a sun#012Never stop fighting until you arrive at your destined place - that is, the unique you#012One best book is equal to a hundred good friends, but one good friend is equal to a library#012validate_multithreaded#012||
Oct  8 18:16:11 FE-C-013E3 aesdsocket: Thread 5 closed
Oct  8 18:16:12 FE-C-013E3 aesdsocket: Thread 6 finished, received a total of 23 data from the client after 0.000321 seconds
Oct  8 18:16:12 FE-C-013E3 aesdsocket: Thread 6 closed
Oct  8 18:16:12 FE-C-013E3 aesdsocket: New thread started, now 2 ongoing
Oct  8 18:16:12 FE-C-013E3 aesdsocket: Accepted connection from 127.0.0.1:56962
Oct  8 18:16:12 FE-C-013E3 aesdsocket: Received 18 bytes, wrote 18 bytes into target file
Oct  8 18:16:12 FE-C-013E3 aesdsocket: Read 808 bytes in local file, sent 808 bytes as acknowledgement, ||abcdefg#012hijklmnop#0121234567890#0129876543210#012If you want to shine like a sun, first burn like a sun#012Never stop fighting until you arrive at your destined place - that is, the unique you#012One best book is equal to a hundred good friends, but one good friend is equal to a library#012timestamp:20241008 18:16:05#012If you want to shine like a sun, first burn like a sun#012Never stop fighting until you arrive at your destined place - that is, the unique you#012One best book is equal to a hundred good friends, but one good friend is equal to a library#012If you want to shine like a sun, first burn like a sun#012Never stop fighting until you arrive at your destined place - that is, the unique you#012One best book is equal to a hundred good friends, but one good friend is equal to a library#012validate_multithreaded#012test_socket_timer#012||
Oct  8 18:16:13 FE-C-013E3 aesdsocket: Thread 5 finished, received a total of 18 data from the client after 0.000297 seconds
Oct  8 18:16:15 FE-C-013E3 aesdsocket: Timestamp thread: wrote 28 bytes into target file
Oct  8 18:16:25 FE-C-013E3 aesdsocket: Timestamp thread: wrote 28 bytes into target file
Oct  8 18:16:34 FE-C-013E3 aesdsocket: New thread started, now 3 ongoing
Oct  8 18:16:34 FE-C-013E3 aesdsocket: Accepted connection from 127.0.0.1:33409
Oct  8 18:16:34 FE-C-013E3 aesdsocket: Received 18 bytes, wrote 18 bytes into target file
Oct  8 18:16:34 FE-C-013E3 aesdsocket: Read 882 bytes in local file, sent 882 bytes as acknowledgement, ||abcdefg#012hijklmnop#0121234567890#0129876543210#012If you want to shine like a sun, first burn like a sun#012Never stop fighting until you arrive at your destined place - that is, the unique you#012One best book is equal to a hundred good friends, but one good friend is equal to a library#012timestamp:20241008 18:16:05#012If you want to shine like a sun, first burn like a sun#012Never stop fighting until you arrive at your destined place - that is, the unique you#012One best book is equal to a hundred good friends, but one good friend is equal to a library#012If you want to shine like a sun, first burn like a sun#012Never stop fighting until you arrive at your destined place - that is, the unique you#012One best book is equal to a hundred good friends, but one good friend is equal to a library#012validate_multithreaded#012test_socket_timer#012timestamp:20241008 18:16:15#012timestamp:20241008 18:16:25#012test_socket_timer#012||
Oct  8 18:16:34 FE-C-013E3 aesdsocket: Thread 5 closed
Oct  8 18:16:35 FE-C-013E3 aesdsocket: Timestamp thread: wrote 28 bytes into target file
Oct  8 18:16:35 FE-C-013E3 aesdsocket: Thread 6 finished, received a total of 18 data from the client after 0.000449 seconds
Oct  8 18:16:35 FE-C-013E3 aesdsocket: Thread 6 closed
Oct  8 18:16:45 FE-C-013E3 aesdsocket: Timestamp thread: wrote 28 bytes into target file
Oct  8 18:16:50 FE-C-013E3 aesdsocket: Received interrupt signal, ending connection
Oct  8 18:16:55 FE-C-013E3 aesdsocket: Timestamp thread: wrote 28 bytes into target file
Oct  8 18:16:55 FE-C-013E3 aesdsocket: Exiting the socket server program, 0 thread still active

# QEMU
Check cat /var/log/messages for seeing socket server outputs

Jan  6 21:53:22 buildroot kern.alert kernel: Hello, world sebaleme
Jan  6 21:53:22 buildroot user.info aesdsocket: Entering server socket program
Jan  6 21:53:22 buildroot daemon.info : starting pid 171, tty '/dev/console': '/sbin/getty -L  console 0 vt100 '
Jan  6 21:53:22 buildroot user.info aesdsocket: Socket created and binded, with file descriptor 4
Jan  6 21:53:22 buildroot user.info aesdsocket: Listening to connections on 4
Jan  6 21:53:27 buildroot auth.info login[171]: root login on 'console'
Jan  6 21:53:35 buildroot user.info aesdsocket: New thread started, now 1 ongoing
Jan  6 21:53:35 buildroot user.info aesdsocket: Accepted connection from 10.0.2.2:39134
Jan  6 21:53:35 buildroot user.info aesdsocket: Received 8 bytes, wrote 8 bytes into target file
Jan  6 21:53:35 buildroot kern.debug kernel: aesdchar: open
Jan  6 21:53:35 buildroot kern.debug kernel: aesdchar: aesd_ioctl called with command : 21505, only valid command is : 3221755393
Jan  6 21:53:35 buildroot kern.debug kernel: aesdchar: Request write 8 bytes with offset 0
Jan  6 21:53:35 buildroot kern.debug kernel: aesdchar: Starting a new write session
Jan  6 21:53:35 buildroot kern.debug kernel: aesdchar: Found EOL char at pos 7 
Jan  6 21:53:35 buildroot kern.debug kernel: aesdchar: Writing element swrite1
Jan  6 21:53:35 buildroot kern.debug kernel: ^H�a^B����^H�a^B����^P^D3^H���� at write pointer 0, read pointer is 0
Jan  6 21:53:35 buildroot kern.debug kernel: aesdchar: Writen swrite1
Jan  6 21:53:35 buildroot kern.debug kernel: ^H�a^B����^H�a^B����^P^D3^H���� at write pointer 0
Jan  6 21:53:35 buildroot kern.debug kernel: aesdchar: 8 bytes were written, new total size is 8
Jan  6 21:53:35 buildroot kern.debug kernel: aesdchar: release
Jan  6 21:53:35 buildroot kern.debug kernel: aesdchar: open
Jan  6 21:53:35 buildroot kern.debug kernel: aesdchar: aesd_ioctl called with command : 21505, only valid command is : 3221755393
Jan  6 21:53:35 buildroot kern.debug kernel: aesdchar: Request 50000 bytes read with offset 0 for entry 0
Jan  6 21:53:35 buildroot kern.debug kernel: aesdchar: offset is 0, accumulated_length is 0, and string is swrite1
Jan  6 21:53:35 buildroot kern.debug kernel: ^H�a^B����^H�a^B����^P^D3^H����
Jan  6 21:53:35 buildroot kern.debug kernel: aesdchar: Read swrite1
Jan  6 21:53:35 buildroot kern.debug kernel: ^H�a^B����^H�a^B����^P^D3^H����, 8 bytes from entry 0 of the circular buffer
Jan  6 21:53:35 buildroot kern.debug kernel: aesdchar: Returns 8 bytes with new offset 8, new read pointer set to 0
Jan  6 21:53:35 buildroot kern.debug kernel: aesdchar: Request 49992 bytes read with offset 8 for entry 0
Jan  6 21:53:35 buildroot kern.debug kernel: aesdchar: entry_id 1, buffer->entry[entry_id].size 0, and offset is 0
Jan  6 21:53:35 buildroot kern.debug kernel: aesdchar: No entrie was written yet, so do nothing
Jan  6 21:53:35 buildroot kern.debug kernel: aesdchar: Returns 0 bytes with new offset 8, new read pointer set to 0
Jan  6 21:53:35 buildroot user.info aesdsocket: Read 8 bytes in local file, sent 8 bytes as acknowledgement, ||swrite1 ||
Jan  6 21:53:36 buildroot user.info aesdsocket: Thread 5 finished, received a total of 8 data from the client after 0.010000 seconds
Jan  6 21:53:36 buildroot user.info aesdsocket: New thread started, now 2 ongoing
Jan  6 21:53:36 buildroot user.info aesdsocket: Accepted connection from 10.0.2.2:41694
Jan  6 21:53:36 buildroot user.info aesdsocket: Received 8 bytes, wrote 8 bytes into target file
Jan  6 21:53:36 buildroot user.info aesdsocket: Read 16 bytes in local file, sent 16 bytes as acknowledgement, ||swrite1 swrite2 ||
Jan  6 21:53:36 buildroot kern.debug kernel: aesdchar: release
Jan  6 21:53:36 buildroot kern.debug kernel: aesdchar: open
Jan  6 21:53:36 buildroot kern.debug kernel: aesdchar: aesd_ioctl called with command : 21505, only valid command is : 3221755393
Jan  6 21:53:36 buildroot kern.debug kernel: aesdchar: Request write 8 bytes with offset 0
Jan  6 21:53:36 buildroot kern.debug kernel: aesdchar: Starting a new write session
Jan  6 21:53:36 buildroot kern.debug kernel: aesdchar: Found EOL char at pos 7 
Jan  6 21:53:36 buildroot kern.debug kernel: aesdchar: Writing element swrite2
Jan  6 21:53:36 buildroot kern.debug kernel:  at write pointer 1, read pointer is 0
Jan  6 21:53:36 buildroot kern.debug kernel: aesdchar: Writen swrite2
Jan  6 21:53:36 buildroot kern.debug kernel:  at write pointer 1
Jan  6 21:53:36 buildroot kern.debug kernel: aesdchar: 8 bytes were written, new total size is 16
Jan  6 21:53:36 buildroot kern.debug kernel: aesdchar: release
Jan  6 21:53:36 buildroot kern.debug kernel: aesdchar: open
Jan  6 21:53:36 buildroot kern.debug kernel: aesdchar: aesd_ioctl called with command : 21505, only valid command is : 3221755393
Jan  6 21:53:36 buildroot kern.debug kernel: aesdchar: Request 50000 bytes read with offset 0 for entry 0
Jan  6 21:53:36 buildroot kern.debug kernel: aesdchar: offset is 0, accumulated_length is 0, and string is swrite1
Jan  6 21:53:36 buildroot kern.debug kernel: ^H�a^B����^H�a^B����^P^D3^H����
Jan  6 21:53:36 buildroot kern.debug kernel: aesdchar: Read swrite1
Jan  6 21:53:36 buildroot kern.debug kernel: ^H�a^B����^H�a^B����^P^D3^H����, 8 bytes from entry 0 of the circular buffer
Jan  6 21:53:36 buildroot kern.debug kernel: aesdchar: Returns 8 bytes with new offset 8, new read pointer set to 0
Jan  6 21:53:36 buildroot kern.debug kernel: aesdchar: Request 49992 bytes read with offset 8 for entry 0
Jan  6 21:53:36 buildroot kern.debug kernel: aesdchar: entry_id 1, buffer->entry[entry_id].size 8, and offset is 0
Jan  6 21:53:36 buildroot kern.debug kernel: aesdchar: offset is 0, accumulated_length is 8, and string is swrite2
Jan  6 21:53:36 buildroot kern.debug kernel: aesdchar: Read swrite2
Jan  6 21:53:36 buildroot kern.debug kernel: , 8 bytes from entry 0 of the circular buffer
Jan  6 21:53:36 buildroot kern.debug kernel: aesdchar: Returns 8 bytes with new offset 16, new read pointer set to 0
Jan  6 21:53:36 buildroot kern.debug kernel: aesdchar: Request 49984 bytes read with offset 16 for entry 0
Jan  6 21:53:36 buildroot kern.debug kernel: aesdchar: entry_id 1, buffer->entry[entry_id].size 8, and offset is 8
Jan  6 21:53:36 buildroot kern.debug kernel: aesdchar: entry_id 2, buffer->entry[entry_id].size 0, and offset is 0
Jan  6 21:53:36 buildroot kern.debug kernel: aesdchar: No entrie was written yet, so do nothing
Jan  6 21:53:36 buildroot kern.debug kernel: aesdchar: Returns 0 bytes with new offset 16, new read pointer set to 0
Jan  6 21:53:36 buildroot user.info aesdsocket: Thread 5 closed
Jan  6 21:53:37 buildroot user.info aesdsocket: Thread 6 finished, received a total of 8 data from the client after 0.000000 seconds
Jan  6 21:53:37 buildroot user.info aesdsocket: Thread 6 closed
Jan  6 21:53:37 buildroot user.info aesdsocket: New thread started, now 1 ongoing
Jan  6 21:53:37 buildroot user.info aesdsocket: Accepted connection from 10.0.2.2:43230
Jan  6 21:53:37 buildroot user.info aesdsocket: Received 8 bytes, wrote 8 bytes into target file
Jan  6 21:53:37 buildroot user.info aesdsocket: Read 24 bytes in local file, sent 24 bytes as acknowledgement, ||swrite1 swrite2 swrite3 ||
Jan  6 21:53:37 buildroot kern.debug kernel: aesdchar: release
Jan  6 21:53:37 buildroot kern.debug kernel: aesdchar: open
Jan  6 21:53:37 buildroot kern.debug kernel: aesdchar: aesd_ioctl called with command : 21505, only valid command is : 3221755393
Jan  6 21:53:37 buildroot kern.debug kernel: aesdchar: Request write 8 bytes with offset 0
Jan  6 21:53:37 buildroot kern.debug kernel: aesdchar: Starting a new write session
Jan  6 21:53:37 buildroot kern.debug kernel: aesdchar: Found EOL char at pos 7 
Jan  6 21:53:37 buildroot kern.debug kernel: aesdchar: Writing element swrite3
Jan  6 21:53:37 buildroot kern.debug kernel:  at write pointer 2, read pointer is 0
Jan  6 21:53:37 buildroot kern.debug kernel: aesdchar: Writen swrite3
Jan  6 21:53:37 buildroot kern.debug kernel:  at write pointer 2
Jan  6 21:53:37 buildroot kern.debug kernel: aesdchar: 8 bytes were written, new total size is 24
Jan  6 21:53:37 buildroot kern.debug kernel: aesdchar: release
Jan  6 21:53:37 buildroot kern.debug kernel: aesdchar: open
Jan  6 21:53:37 buildroot kern.debug kernel: aesdchar: aesd_ioctl called with command : 21505, only valid command is : 3221755393
Jan  6 21:53:37 buildroot kern.debug kernel: aesdchar: Request 50000 bytes read with offset 0 for entry 0
Jan  6 21:53:37 buildroot kern.debug kernel: aesdchar: offset is 0, accumulated_length is 0, and string is swrite1
Jan  6 21:53:37 buildroot kern.debug kernel: ^H�a^B����^H�a^B����^P^D3^H����
Jan  6 21:53:37 buildroot kern.debug kernel: aesdchar: Read swrite1
Jan  6 21:53:37 buildroot kern.debug kernel: ^H�a^B����^H�a^B����^P^D3^H����, 8 bytes from entry 0 of the circular buffer
Jan  6 21:53:37 buildroot kern.debug kernel: aesdchar: Returns 8 bytes with new offset 8, new read pointer set to 0
Jan  6 21:53:37 buildroot kern.debug kernel: aesdchar: Request 49992 bytes read with offset 8 for entry 0
Jan  6 21:53:37 buildroot kern.debug kernel: aesdchar: entry_id 1, buffer->entry[entry_id].size 8, and offset is 0
Jan  6 21:53:37 buildroot kern.debug kernel: aesdchar: offset is 0, accumulated_length is 8, and string is swrite2
Jan  6 21:53:37 buildroot kern.debug kernel: aesdchar: Read swrite2
Jan  6 21:53:37 buildroot kern.debug kernel: , 8 bytes from entry 0 of the circular buffer
Jan  6 21:53:37 buildroot kern.debug kernel: aesdchar: Returns 8 bytes with new offset 16, new read pointer set to 0
Jan  6 21:53:37 buildroot kern.debug kernel: aesdchar: Request 49984 bytes read with offset 16 for entry 0
Jan  6 21:53:37 buildroot kern.debug kernel: aesdchar: entry_id 1, buffer->entry[entry_id].size 8, and offset is 8
Jan  6 21:53:37 buildroot kern.debug kernel: aesdchar: entry_id 2, buffer->entry[entry_id].size 8, and offset is 0
Jan  6 21:53:37 buildroot kern.debug kernel: aesdchar: offset is 0, accumulated_length is 16, and string is swrite3
Jan  6 21:53:37 buildroot kern.debug kernel: aesdchar: Read swrite3
Jan  6 21:53:37 buildroot kern.debug kernel: , 8 bytes from entry 0 of the circular buffer
Jan  6 21:53:37 buildroot kern.debug kernel: aesdchar: Returns 8 bytes with new offset 24, new read pointer set to 0
Jan  6 21:53:37 buildroot kern.debug kernel: aesdchar: Request 49976 bytes read with offset 24 for entry 0
Jan  6 21:53:37 buildroot kern.debug kernel: aesdchar: entry_id 1, buffer->entry[entry_id].size 8, and offset is 16
Jan  6 21:53:37 buildroot kern.debug kernel: aesdchar: entry_id 2, buffer->entry[entry_id].size 8, and offset is 8
Jan  6 21:53:37 buildroot kern.debug kernel: aesdchar: entry_id 3, buffer->entry[entry_id].size 0, and offset is 0
Jan  6 21:53:37 buildroot kern.debug kernel: aesdchar: No entrie was written yet, so do nothing
Jan  6 21:53:37 buildroot kern.debug kernel: aesdchar: Returns 0 bytes with new offset 24, new read pointer set to 0
Jan  6 21:53:38 buildroot user.info aesdsocket: Thread 5 finished, received a total of 8 data from the client after 0.000000 seconds
Jan  6 21:53:38 buildroot user.info aesdsocket: New thread started, now 2 ongoing
Jan  6 21:53:38 buildroot user.info aesdsocket: Accepted connection from 10.0.2.2:45790
Jan  6 21:53:38 buildroot user.info aesdsocket: The request is a IOCTL command to set the read pointer
Jan  6 21:53:38 buildroot user.info aesdsocket: Entering ioctl, working with 0,2
Jan  6 21:53:38 buildroot user.info aesdsocket: Found X = 0 and Y = 2
Jan  6 21:53:38 buildroot user.info aesdsocket: Read 0 bytes in local file, sent 0 bytes as acknowledgement, ||||
Jan  6 21:53:38 buildroot kern.debug kernel: aesdchar: release
Jan  6 21:53:38 buildroot kern.debug kernel: aesdchar: open
Jan  6 21:53:38 buildroot kern.debug kernel: aesdchar: aesd_ioctl called with command : 21505, only valid command is : 3221755393
Jan  6 21:53:38 buildroot kern.debug kernel: aesdchar: aesd_ioctl called with command : 3221755393, only valid command is : 3221755393
Jan  6 21:53:38 buildroot kern.debug kernel: aesdchar: Adjusting file offset with: 0 and offset 2
Jan  6 21:53:38 buildroot kern.debug kernel: aesdchar: File offset set to: 2
Jan  6 21:53:38 buildroot user.info aesdsocket: Thread 5 closed
Jan  6 21:53:39 buildroot user.info aesdsocket: Thread 6 finished, received a total of 23 data from the client after 0.000000 seconds
Jan  6 21:53:39 buildroot user.info aesdsocket: Thread 6 closed
# cat /var/log/messages
