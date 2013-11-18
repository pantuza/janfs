/**
 * TCP socket listener
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>

#include "listener.h"


void fill_values(struct sockaddr_in *addr)
{
    bzero(addr, sizeof(*addr));
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = htonl(INADDR_ANY);
    addr->sin_port = htonl(SRV_PORT);
}

void tcp_listener()
{
    int sockfd, connfd;
    struct sockaddr_in srvaddr, cliaddr;
    socklen_t clilen;

    char clibuff[BUFFER_SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    fill_values(&srvaddr);
    bind(sockfd, (struct sockaddr *) &srvaddr, sizeof(srvaddr));

    listen(sockfd, BUFFER_SIZE);

    clilen = sizeof(cliaddr);
    connfd = accept(sockfd, (struct sockaddr *) &cliaddr, &clilen);

    while (1) {
    
        /* TODO:  Receive connections and call local file system */
    }

    close(connfd);
}
