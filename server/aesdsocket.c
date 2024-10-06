/* Copyright (c) 2024 Sebastien Lemetter
 * aesdsocket.c: Create a socket connection
 * ========================================== */
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <syslog.h>


#include "aesdsocket.h"

#define BACKLOG 10
#define FILEPATH "/var/tmp/aesdsocketdata"
#define BUFFER_SIZE 2000


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
    // Total number of bytes and char array to be sent back to client
    int32_t len = 0;
    char sendBuffer[60000] = {0};

    // Listen and accept connections
    struct addrinfo *my_addr = NULL;
    int socket_fd = createSocketConnection(my_addr);
    listen(socket_fd, BACKLOG);
    syslog(LOG_INFO, "Listening to connections on %d\n", socket_fd);
    struct CThreadInstance thread;
    thread.addr_size = sizeof thread.client_addr;

    // Accept returns "fd" which is the socket file descriptor for the accepted connection,
    // and socket_fd remains the socket file descriptor, still listening for other connections
    // fd is the accepted socket, and will be used for sending/receiving data
    while((thread.fd = accept(socket_fd, (struct sockaddr *)&(thread.client_addr), &(thread.addr_size))))
    {
        // Extracting client IP address from the socket address storage
        struct sockaddr_in *sin = (struct sockaddr_in *)&(thread.client_addr);
        unsigned char *client_ip = (unsigned char *)&sin->sin_addr.s_addr;
        unsigned short int client_port = sin->sin_port;
        if (thread.fd != -1)
        {
            syslog(LOG_INFO, "Accepted connection from %d.%d.%d.%d:%d\n", client_ip[0], client_ip[1], client_ip[2], client_ip[3], client_port);
        }

        // Create a new file to store the received packages
        FILE *file = NULL;
        file = fopen(FILEPATH, "a");
        if (file == NULL)
        {
            syslog(LOG_ERR, "Value of errno attempting to open file %s: %d\n", FILEPATH, errno);
            return 1;
        }

        // Receive data from open port
        char* buffer = malloc(BUFFER_SIZE);
        while (true)
        {
            memset(buffer, 0x00, BUFFER_SIZE);
            int bytes_num = recv(thread.fd, buffer, BUFFER_SIZE, 0);
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
            int written_bytes = fwrite(buffer, sizeof(char), bytes_num, file);
            syslog(LOG_INFO, "Received %d bytes, full content is %d bytes, wrote %d bytes into target file\n", bytes_num, len, written_bytes);
            // Prepare sendBuffer
            strcat(sendBuffer, buffer);

            // If new line character, this is the last package and send the answer
            if(memchr(buffer, '\n', bytes_num) != NULL) {
                // Send the full received content as acknowledgement
                int bytes_sent = send(thread.fd, sendBuffer, len, 0);
                syslog(LOG_INFO, "Sent %d bytes as acknowledgement, ||%s||\n", bytes_sent, sendBuffer);
                if (bytes_sent == -1)
                {
                    syslog(LOG_ERR, "Value of errno attempting to send data to %d.%d.%d.%d: %d\n", client_ip[0], client_ip[1], client_ip[2], client_ip[3], errno);
                    break;
                }
            }
        }

        fclose(file);
        syslog(LOG_INFO, "Closed connection from %d.%d.%d.%d:%d\n", client_ip[0], client_ip[1], client_ip[2], client_ip[3], client_port);
    }

    // Free my_addr once we are finished and close the remaining file descriptors
    free(my_addr);
    close(thread.fd);
    close(socket_fd);
    closelog();

    return 0;
}
