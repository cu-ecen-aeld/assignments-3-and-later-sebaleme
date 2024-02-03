/* Copyright (c) 2024 Sebastien Lemetter
 * aesdsocket.c: Create a socket connection
 * ========================================== */
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <syslog.h>

#define PORT "9000"
#define BACKLOG 10
#define FILEPATH "/var/tmp/aesdsocketdata"


int main()
{
    // Create socket and bind it to given port
    int socket_fd = socket(PF_INET, SOCK_STREAM, 0);

    struct addrinfo *my_addr;
    // first, load up address structs with getaddrinfo():
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;  // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
    int status = getaddrinfo(NULL, PORT, &hints, &my_addr);
    if(status != 0 || my_addr == NULL)
    {
        return -1;
    }

    // Assign an address to the socket
    bind(socket_fd, my_addr->ai_addr, sizeof(struct sockaddr));

    // Listen and accept connections
    struct sockaddr_storage client_addr;
    listen(socket_fd, BACKLOG);
    socklen_t addr_size = sizeof client_addr;
    // Accept returns "fd" which is the socket file descriptor for the accepted connection,
    // and socket_fd remains the socket file descriptor, still listening for other connections
    int fd = accept(socket_fd, (struct sockaddr *)&client_addr, &addr_size);

    // Extracting client IP address from the socket address storage
    struct sockaddr_in *sin = (struct sockaddr_in *)&client_addr;
    unsigned char *ip = (unsigned char *)&sin->sin_addr.s_addr;

    if(fd)
    {
        syslog(LOG_INFO, "Accepted connection from %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    }

    // Create a new file to store the received packages
    FILE *fp = NULL;
    fp = fopen(FILEPATH ,"w");
    if(fp == NULL)
    {
        syslog(LOG_ERR, "Value of errno attempting to open file %s: %d\n", FILEPATH, errno);
        return 1;
    }

    // Receive data from open port
    char buffer[200] = { 0 };
    while(true)
    {   
        int bytes_num = recv(socket_fd, buffer, 200, 0);
        if (bytes_num == 0)
        {
            break;
        }
        if (bytes_num == -1)
        {
            syslog(LOG_ERROR, "Value of errno attempting to receive data from %d.%d.%d.%d: %d\n", ip[0], ip[1], ip[2], ip[3], errno);
            break;
        }
        // Store the last received packet in target file
        fprintf( fp, buffer);
        // Send the full received content as acknowledgement
        int bytes_sent = send(socket_fd, buffer, 200, 0);
    }
    syslog(LOG_INFO, "Closed connection from %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

    // Free my_addr once we are finished
    free(my_addr);

    return 0;
}
