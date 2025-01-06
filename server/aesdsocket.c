/* Copyright (c) 2024 Sebastien Lemetter
 * aesdsocket.c: Create a socket connection
 * ========================================== */
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <syslog.h>
#include <time.h>

#include "../aesd-char-driver/aesd_ioctl.h"
#include "aesdsocket.h"
#include "queue_bsd.h"

#define BACKLOG 10
#define FILEPATH "/dev/aesdchar"
#define BUFFER_SIZE 2000
#define MAX_BUFFER_SIZE 50000
#define TIMEGAP_USECOND 10000000 //10s
#define WAIT_DELAY 10000 //10ms

/// Thread function writing regular timestamps
void* timestamp_func(void* thread_ts_param)
{
    time_t t;
    struct tm *tmp;

    // Cast input param back to a useful type
    struct CThreadInstance* ts_data = (struct CThreadInstance *) thread_ts_param;

    while(keepRunning)
    {
        usleep(TIMEGAP_USECOND);

        // Create required locals for formatted timestamp. Need to be initialized at each iteration
        char tsstr[200] = "timestamp:";
        char outstr[200];

        //Format timestamp
        t = time(NULL);
        tmp = localtime(&t);
        int bytes_num = strftime(outstr, sizeof(outstr), "%Y%m%d %H:%M:%S", tmp);
        strcat(tsstr, outstr);
        strcat(tsstr, "\n");

        // Write data to file
        get_mutex(ts_data->file_mutex);
        FILE *file = fopen(FILEPATH, "a");
        if (file == NULL)
        {
            syslog(LOG_ERR, "Value of errno attempting to open file %s: %d\n", FILEPATH, errno);
            break;
        }
        // +11 because tsstr was appended with the new line character and outstr
        int written_bytes = fwrite(tsstr, sizeof(char), bytes_num+11, file);
        fclose(file);
        release_mutex(ts_data->file_mutex);
        syslog(LOG_INFO, "Timestamp thread: wrote %d bytes into target file\n", written_bytes);
    }
    return thread_ts_param;
}

// Prepare and execute ioctl syscall
int run_ioctl_command(const char *p, int fd)
{
    syslog(LOG_INFO, "Entering ioctl, working with %s\n", p);
    char *endptr;
    struct aesd_seekto seekto;

    // Now we are looking for X (command) and Y (offset)
    errno = 0;  // Clear errno before the call
    seekto.write_cmd = strtoul(p, &endptr, 10);
    if (errno == ERANGE)
    {
        errno = 0;
        syslog(LOG_ERR, "Could not convert X value to an unsigned int");
    }

    // Parse the second number (Y)
    p = endptr + 1; // Move past the comma
    seekto.write_cmd_offset = strtoul(p, &endptr, 10);
    if (errno == ERANGE)
    {
        errno = 0;
        syslog(LOG_ERR, "Could not convert Y value to an unsigned int");
    }

    syslog(LOG_INFO,"Found X = %u and Y = %u", seekto.write_cmd, seekto.write_cmd_offset);

    // Here we bypass the ioctl function, and directly update the f_pos pointer.
    return ioctl(fd, AESDCHAR_IOCSEEKTO, &seekto);
}

/// Thread processes new transmission
void* threadfunc(void* thread_param)
{
    int fd;
    char *p;
    const char *prefix = "AESDCHAR_IOCSEEKTO:";
    clock_t start = clock();

    // Cast input param back to a useful type
    struct CThreadInstance* data = (struct CThreadInstance *) thread_param;

    // Receive data from open port
    char* buffer = malloc(BUFFER_SIZE);

    // Total number of bytes received by this thread
    int32_t len = 0;

    // Extracting client IP address from the socket address storage
    struct sockaddr_in *sin = (struct sockaddr_in *)&(data->client_addr);
    unsigned char *client_ip = (unsigned char *)&sin->sin_addr.s_addr;
    unsigned short int client_port = sin->sin_port;
    if (data->fd != -1)
    {
        syslog(LOG_INFO, "Accepted connection from %d.%d.%d.%d:%d\n", client_ip[0], client_ip[1], client_ip[2], client_ip[3], client_port);
    }

    while (keepRunning)
    {
        memset(buffer, 0x00, BUFFER_SIZE);
        int bytes_num = recv(data->fd, buffer, BUFFER_SIZE, 0);
        if (bytes_num == 0)
        {
            // 0 byte received, the connection was closed by the client
            break;
        }
        if (bytes_num == -1)
        {
            // Errno 107 means that the socket is NOT connected (any more)
            syslog(LOG_ERR, "Value of errno attempting to receive data from %d.%d.%d.%d: %d\n", client_ip[0], client_ip[1], client_ip[2], client_ip[3], errno);
            //printf("Could not receive data from client, ending receiving, errno is %d\n", errno);
            break;
        }
        // Store the last received packet in target file
        len += bytes_num;
        get_mutex(data->file_mutex);
        FILE *file = fopen(FILEPATH, "a");
        if (file == NULL)
        {
            syslog(LOG_ERR, "Value of errno attempting to open file %s: %d\n", FILEPATH, errno);
            break;
        }

        // If received string is an IOCTL, special handling. In particular, do not close file
        // between ioctl request and read action
        if (strstr(buffer, prefix) != NULL) {
            syslog(LOG_INFO, "The request is a IOCTL command to set the read pointer");

            // Move past the prefix and run the ioctl command
            p = buffer + strlen(prefix);
            fd = fileno(file);
            run_ioctl_command(p, fd);

            // Then read the content, close the file and release the mutex
            char sendBuffer[MAX_BUFFER_SIZE] = {0};
            int read_bytes = fread(sendBuffer, sizeof(char), MAX_BUFFER_SIZE, file);
            fclose(file);
            release_mutex(data->file_mutex);
            // Send the full received content as acknowledgement
            int bytes_sent = send(data->fd, sendBuffer, read_bytes, 0);
            syslog(LOG_INFO, "Read %d bytes in local file, sent %d bytes as acknowledgement, ||%s||\n", read_bytes, bytes_sent, sendBuffer);
        }
        else
        {
            int written_bytes = fwrite(buffer, sizeof(char), bytes_num, file);
            fclose(file);
            release_mutex(data->file_mutex);
            syslog(LOG_INFO, "Received %d bytes, wrote %d bytes into target file\n", bytes_num, written_bytes);

            // If new line character, this is the last package and send the answer
            if(memchr(buffer, '\n', bytes_num) != NULL) {
                // Prepare sendBuffer, containing the answer to the client
                char sendBuffer[MAX_BUFFER_SIZE] = {0};
                get_mutex(data->file_mutex);
                FILE *fileRead = fopen(FILEPATH, "r");
                int read_bytes = fread(sendBuffer, sizeof(char), MAX_BUFFER_SIZE, file);
                fclose(fileRead);
                release_mutex(data->file_mutex);
                // Send the full received content as acknowledgement
                int bytes_sent = send(data->fd, sendBuffer, read_bytes, 0);
                syslog(LOG_INFO, "Read %d bytes in local file, sent %d bytes as acknowledgement, ||%s||\n", read_bytes, bytes_sent, sendBuffer);
                if (bytes_sent == -1)
                {
                    syslog(LOG_ERR, "Value of errno attempting to send data to %d.%d.%d.%d: %d\n", client_ip[0], client_ip[1], client_ip[2], client_ip[3], errno);
                    break;
                }
            }
        }
    }
    
    clock_t end = clock();
    float seconds = (float)(end - start) / CLOCKS_PER_SEC;
    syslog(LOG_INFO, "Thread %d finished, received a total of %d data from the client after %f seconds\n", data->fd, len, seconds);
    free(buffer);
    // No need of mutex since operation is atomic.
    data->done = true;
    return thread_param;
}

int main(int argc, char** argv)
{
    // Use the syslog for non interactive application
    openlog("aesdsocket",0,LOG_USER);
    syslog(LOG_INFO, "Entering server socket program\n");

    // Capture abort signal
    signal(SIGINT, intHandler);
    signal(SIGTERM, intHandler);

    // Run process as daemon if called with option -d
    if((argc == 2)&&(argv[1][0] == '-')&&(argv[1][1] == 'd'))
    {
        create_deamon();
    }

    // Listen and accept connections
    struct addrinfo *my_addr = NULL;
    socket_fd = createSocketConnection(&my_addr);
    listen(socket_fd, BACKLOG);
    syslog(LOG_INFO, "Listening to connections on %d\n", socket_fd);
    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof(struct sockaddr_storage); // Address size, depends of address type (IPv4 or IPv6)
    // Create thread queue, using singly Linked List
    SLIST_HEAD(slisthead, slist_data_s) head;
    SLIST_INIT(&head);
    int sizeQ = 0;

    // Create mutex for file synchronisation between all the threads (they will all access the same resource)
    pthread_mutex_t file_mutex;
    pthread_mutex_init(&file_mutex, NULL);

    // We remove TS printing in assignment 8
    // Create new thread for writing timestamps
    //pthread_t ts_thread;
    //struct CThreadInstance* ts_thread_data =  malloc(sizeof(struct CThreadInstance));
    //ts_thread_data->thread = &ts_thread;
    //ts_thread_data->file_mutex = &file_mutex;
    //int rc = pthread_create(&ts_thread, NULL, timestamp_func, ts_thread_data);
    // Need to free the dynamic allocated struct if pthread creation fails
    //if(rc != 0)
    //{
    //    syslog(LOG_INFO, "timestamp thread could not be started: %d\n", errno);
    //}
    //else
    //{
    //    syslog(LOG_INFO, "timestamp thread started, now %d ongoing\n", ++sizeQ);
    //}

    while(keepRunning)
    {
        // Accept returns "fd" which is the socket file descriptor for the accepted connection,
        // and socket_fd remains the socket file descriptor, still listening for other connections
        // fd is the accepted socket, and will be used for sending/receiving data
        // Accept is a blocking function, but closing the socket exits the function
        int fd = accept(socket_fd, (struct sockaddr *)&(client_addr), &(addr_size));
        // If accept() unklocked by an abort signal, no accepted connection thread should be started
        if(fd >= 0)
        {
            // Create new thread
            pthread_t thread;

            // Prepare thread context
            struct slist_data_s *slist_data_ptr = malloc(sizeof(struct slist_data_s));
            slist_data_ptr->thread_data.fd = fd;
            slist_data_ptr->thread_data.done = false;
            slist_data_ptr->thread_data.client_addr = client_addr;
            slist_data_ptr->thread_data.thread = &thread;
            slist_data_ptr->thread_data.file_mutex = &file_mutex;

            if(head.slh_first == NULL)
            {
                SLIST_INSERT_HEAD(&head, slist_data_ptr, pointers);
            }
            else
            {
                // Insert new element after the head. No need to have any specific order
                SLIST_INSERT_AFTER(head.slh_first,slist_data_ptr, pointers);
            }

            // Start thread with its internal data
            struct CThreadInstance* data =  &(slist_data_ptr->thread_data);
            int rc = pthread_create(&thread, NULL, threadfunc, data);
            syslog(LOG_INFO, "New thread started, now %d ongoing\n", ++sizeQ);
            // Need to free the dynamic allocated struct if pthread creation fails
            if(rc != 0)
            {
                free(slist_data_ptr);
                // Transmission is lost, directly waiting for next connection
                continue;
            }
        }

        // Remove finished thread if any and release related allocated memory
        // Give time to the thread to finish first
        usleep(WAIT_DELAY);
        struct slist_data_s *elementP, *elementPTemp;
        SLIST_FOREACH_SAFE(elementP, &head, pointers, elementPTemp)
            if(elementP->thread_data.done)
            {
                // Close thread, no return value
                syslog(LOG_INFO, "Thread %d closed\n", elementP->thread_data.fd);
                pthread_join(*(elementP->thread_data.thread),NULL);
                close(elementP->thread_data.fd);
                SLIST_REMOVE(&head, elementP, slist_data_s, pointers);
                free(elementP);
                sizeQ--;
            }
    }

    // Terminate the timestamp thread
    //pthread_join(ts_thread,NULL);
    //free(ts_thread_data);
    sizeQ--;
    syslog(LOG_INFO, "Exiting the socket server program, %d thread still active\n", sizeQ);
    usleep(WAIT_DELAY);
    // Free my_addr once we are finished
    freeaddrinfo(my_addr);
    closelog();

    return 0;
}
