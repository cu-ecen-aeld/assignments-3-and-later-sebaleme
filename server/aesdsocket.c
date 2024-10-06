/* Copyright (c) 2024 Sebastien Lemetter
 * aesdsocket.c: Create a socket connection
 * ========================================== */
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <syslog.h>
#include <time.h>

#include "aesdsocket.h"
#include "queue_bsd.h"

#define BACKLOG 10
#define FILEPATH "/var/tmp/aesdsocketdata"
#define BUFFER_SIZE 2000

/// Thread processes new transmission
void* threadfunc(void* thread_param)
{
    clock_t start = clock();

    // Cast input param back to a useful type
    struct thread_data* data = (struct thread_data *) thread_param;

    // Receive data from open port
    char* buffer = malloc(BUFFER_SIZE);

    // Total number of bytes and char array to be sent back to client
    int32_t len = 0;
    char sendBuffer[60000] = {0};

        // Extracting client IP address from the socket address storage
    struct sockaddr_in *sin = (struct sockaddr_in *)&(data->client_addr);
    unsigned char *client_ip = (unsigned char *)&sin->sin_addr.s_addr;
    unsigned short int client_port = sin->sin_port;
    if (data->fd != -1)
    {
        syslog(LOG_INFO, "Accepted connection from %d.%d.%d.%d:%d\n", client_ip[0], client_ip[1], client_ip[2], client_ip[3], client_port);
    }

    clock_t end = clock();
    float seconds = (float)(end - start) / CLOCKS_PER_SEC;
    syslog(LOG_INFO, "Closed connection from %d.%d.%d.%d:%d after %f seconds\n", client_ip[0], client_ip[1], client_ip[2], client_ip[3], client_port, seconds);

    while (true)
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
        data->file = fopen(FILEPATH, "a");
        int written_bytes = fwrite(buffer, sizeof(char), bytes_num, data->file);
        fclose(data->file);
        syslog(LOG_INFO, "Received %d bytes, full content is %d bytes, wrote %d bytes into target file\n", bytes_num, len, written_bytes);
        // Prepare sendBuffer
        strcat(sendBuffer, buffer);

        // If new line character, this is the last package and send the answer
        if(memchr(buffer, '\n', bytes_num) != NULL) {
            // Send the full received content as acknowledgement
            int bytes_sent = send(data->fd, sendBuffer, len, 0);
            syslog(LOG_INFO, "Sent %d bytes as acknowledgement, ||%s||\n", bytes_sent, sendBuffer);
            if (bytes_sent == -1)
            {
                syslog(LOG_ERR, "Value of errno attempting to send data to %d.%d.%d.%d: %d\n", client_ip[0], client_ip[1], client_ip[2], client_ip[3], errno);
                break;
            }
        }
    }

    free(buffer);
    data->done = true;
    return thread_param;
}

int main(int argc, char** argv)
{
    // Use the syslog for non interactive application
    openlog("aesdsocket",0,LOG_USER);
    syslog(LOG_INFO, "Entering server socket program\n");

    // Run process as daemon if called with option -d
    if((argc == 2)&&(argv[1][0] == '-')&&(argv[1][1] == 'd'))
    {
        create_deamon();
    }

    // Delete output file if already exists
    remove(FILEPATH);

    // Listen and accept connections
    struct addrinfo *my_addr = NULL;
    int socket_fd = createSocketConnection(my_addr);
    listen(socket_fd, BACKLOG);
    syslog(LOG_INFO, "Listening to connections on %d\n", socket_fd);
    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof(struct sockaddr_storage); // Address size, depends of address type (IPv4 or IPv6)
    // Create thread queue, using singly Linked List
    SLIST_HEAD(slisthead, slist_data_s) head;
    SLIST_INIT(&head);
    int sizeQ = 0;

    // Create a new file to store the received packages and its related mutex
    FILE *file = NULL;
    //file = fopen(FILEPATH, "a");
    pthread_mutex_t file_mutex;
    pthread_mutex_init(&file_mutex, NULL);
    if (file == NULL)
    {
        syslog(LOG_ERR, "Value of errno attempting to open file %s: %d\n", FILEPATH, errno);
        return 1;
    }

    // Accept returns "fd" which is the socket file descriptor for the accepted connection,
    // and socket_fd remains the socket file descriptor, still listening for other connections
    // fd is the accepted socket, and will be used for sending/receiving data
    int fd;
    while((fd = accept(socket_fd, (struct sockaddr *)&(client_addr), &(addr_size))))
    {
        // Create new thread
        pthread_t thread;

        // Prepare thread context
        struct slist_data_s *slist_data_ptr = malloc(sizeof(struct slist_data_s));
        syslog(LOG_INFO, "New thread started, now %d ongoing\n", ++sizeQ);
        slist_data_ptr->thread_data.fd = fd;
        slist_data_ptr->thread_data.client_addr = client_addr;
        slist_data_ptr->thread_data.thread = &thread;
        slist_data_ptr->thread_data.file_mutex = file_mutex;

        if(head.slh_first == NULL)
        {
            SLIST_INSERT_HEAD(&head, slist_data_ptr, pointers);
        }
        else
        {
            // Insert new element after the head. No need to have any specific order
            SLIST_INSERT_AFTER(head.slh_first,slist_data_ptr, pointers);
        }

        // Start thread
        int rc = pthread_create(&thread, NULL, threadfunc, slist_data_ptr);
        // Need to free the dynamic allocated struct if pthread creation fails
        if(rc != 0)
        {
            free(slist_data_ptr);
            // Transmission is lost, directly waiting for next connection
            continue;
        }

        // Remove finished thread if any and release related allocated memory
        struct slist_data_s *elementP, *elementPTemp;
        SLIST_FOREACH_SAFE(elementP, &head, pointers, elementPTemp)
            if(elementP->thread_data.done)
                pthread_join(*(elementP->thread_data.thread),&(elementP->thread_data));
                SLIST_REMOVE(&head, elementP, slist_data_s, pointers);
                free(elementP);
                sizeQ--;
    }

    // TODO: Somehow, this log is not printed??
    syslog(LOG_INFO, "There are still %d threads running\n", sizeQ);

    // Free my_addr once we are finished and close the remaining file descriptors
    free(my_addr);
    close(fd);
    close(socket_fd);
    closelog();

    return 0;
}
