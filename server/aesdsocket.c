/* Copyright (c) 2024 Sebastien Lemetter
 * aesdsocket.c: Create a socket connection
 * ========================================== */
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#define PORT "9000"
#define BACKLOG 10

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
    int fd = accept(socket_fd, (struct sockaddr *)&client_addr, &addr_size);

    // Free my_addr once we are finished
    free(my_addr);

    return 0;
}
