#define _GNU_SOURCE
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include  <errno.h>
#include <unistd.h>

/*
* tcp server implementation
*/
#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)
#define PORT 1026
#define SERVER_BACKLOG_SIZE 2


int
main(void)
{   
    int servfd;
    int clientfd;
    struct sockaddr_in serv_addr;
    struct sockaddr_in client_addr;
    socklen_t  client_addr_size;
    //create a tcp ip socket
    servfd = socket(AF_INET, SOCK_STREAM, 0);
    if (servfd == -1){
        handle_error("socket creation error:");
    }
    //bind : the socket ie "give a name to the socket"
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
    if (bind(servfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr))==-1){
        handle_error("Bind error:");
    }
    //listen : marks  the  socket referred to by servfd as a passive socket ie a listening socket
    if (listen(servfd, SERVER_BACKLOG_SIZE) == -1){
        handle_error("Listen error :");
    }

    // accept a connection on a socket
    client_addr_size = sizeof(client_addr);
    clientfd = accept4(servfd, (struct sockaddr *) &client_addr, &client_addr_size,SOCK_NONBLOCK);
    if (clientfd== -1){
        handle_error("accept");
    }

    char buffRsp[21]="ServerNotImplemented";
    char readBuf[4];
    ssize_t nbytes;
    for (;;) {
        if ((nbytes = read(clientfd, readBuf, sizeof(readBuf))) < 0) {
            if (errno != EWOULDBLOCK) {
                perror("Read error:");
            }
        } else {
            write(clientfd, buffRsp, sizeof(buffRsp)); 
        }
    }
               
}

