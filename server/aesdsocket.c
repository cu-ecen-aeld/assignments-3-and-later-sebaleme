/* Copyright (c) 2024 Sebastien Lemetter
 * aesdsocket.c: Create a socket connection
 * ========================================== */
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <syslog.h>
#include <unistd.h>

#define PORT "9000"
#define BACKLOG 10
#define FILEPATH "/var/tmp/aesdsocketdata"
#define BUFFER_SIZE 2000

// (IPv4 only--see struct sockaddr_in6 for IPv6)
// struct sockaddr_in {
//     short int          sin_family;  // Address family, AF_INET
//     unsigned short int sin_port;    // Port number
//     struct in_addr     sin_addr;    // Internet address
//     unsigned char      sin_zero[8]; // Same size as struct sockaddr
// };


/// Function which transfer process to a deamon
/// A deamon is running outside any console, in the system background
void create_deamon()
{        
    // creating child process which is not attached to a TTY
    int pid = fork();
    // An error occurred
    if (pid < 0)
        exit(EXIT_FAILURE);
    // Success: Let the parent terminate so grand parent can proceed
    if (pid > 0)
        exit(EXIT_SUCCESS);

    // Create a new session so daemon is not linked to any tty
    if (setsid() < 0)
        exit(EXIT_FAILURE);

    // Change the working directory to the root directory so no possible error if unmount
    chdir("/");

    // Redirect stdin, stdout and stderr to /dev/null, so our daemon won t communicate through a console
    close(0); close(1); close(2);
    open("/dev/null",O_RDWR); dup(0); dup(0);
}

///
///
int createSocketConnection(struct addrinfo* my_addr)
{
    // Create socket and bind it to given port
    int socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    // first, load up address structs with getaddrinfo():
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // fill in my IP for me
    int status = getaddrinfo(NULL, PORT, &hints, &my_addr);
    if (status != 0 || my_addr == NULL)
    {
        return -1;
    }

    // Assign an address to the socket
    bind(socket_fd, my_addr->ai_addr, sizeof(struct sockaddr));
    syslog(LOG_INFO, "Socket created and binded, with file descriptor %d\n", socket_fd);
    return socket_fd;
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
    // Total number of bytes and char array to be sent back to client
    int32_t len = 0;
    char sendBuffer[60000] = {0};

    // Listen and accept connections
    struct sockaddr_storage client_addr;
    struct addrinfo *my_addr = NULL;
    int socket_fd = createSocketConnection(my_addr);
    listen(socket_fd, BACKLOG);
    syslog(LOG_INFO, "Listening to connections on %d\n", socket_fd);
    socklen_t addr_size = sizeof client_addr;

    // Accept returns "fd" which is the socket file descriptor for the accepted connection,
    // and socket_fd remains the socket file descriptor, still listening for other connections
    // fd is the accepted socket, and will be used for sending/receiving data
    int fd;
    while((fd = accept(socket_fd, (struct sockaddr *)&client_addr, &addr_size)))
    {
        // Extracting client IP address from the socket address storage
        struct sockaddr_in *sin = (struct sockaddr_in *)&client_addr;
        unsigned char *client_ip = (unsigned char *)&sin->sin_addr.s_addr;
        unsigned short int client_port = sin->sin_port;
        if (fd != -1)
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
            int bytes_num = recv(fd, buffer, BUFFER_SIZE, 0);
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
                int bytes_sent = send(fd, sendBuffer, len, 0);
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
    close(fd);
    close(socket_fd);
    closelog();

    return 0;
}
