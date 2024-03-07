#define _GNU_SOURCE
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include  <errno.h>
#include <unistd.h>
#include <poll.h>

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
    int ret;
    struct sockaddr_in serv_addr;
    struct sockaddr_in client_addr;
    socklen_t  client_addr_size;
    struct pollfd  pfds[1];
    int num_open_fds=1;
    char rspBuff[21]="ServerNotImplemented";
    char readBuf[4];

    /*create a tcp ip socket*/
    servfd = socket(AF_INET, SOCK_STREAM, 0);
    if (servfd == -1){
        handle_error("socket creation error:");
    }
    /*bind : the socket ie "give a name to the socket*/
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
    ret=bind(servfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr));
    if (ret==-1){
        handle_error("Bind error:");
    }
    /*listen : marks  the  socket referred to by servfd as a passive socket ie a listening socket*/
    ret=listen(servfd, SERVER_BACKLOG_SIZE);
    if (ret == -1){
        handle_error("Listen error :");
    }
    
    /*polling on fd id=3*/
    pfds[0].fd=3;
    pfds[0].events = POLLIN;
    
    while (num_open_fds==1) {
        /*polling on fd passed as argument, with infinite tmo ie poll is blocked until an event POLLIN occurs*/
        ret=poll(pfds, 1, -1);
        if (ret == -1){
            handle_error("Read error:");
        }
        if (pfds[0].revents != 0) {
            if (pfds[0].revents & POLLIN) {
                client_addr_size = sizeof(client_addr);
                clientfd = accept(servfd, (struct sockaddr *) &client_addr, &client_addr_size);
                ret = read(clientfd, readBuf, sizeof(readBuf));

                if (ret == -1)
                    handle_error("Read error:");
                else{
                    write(clientfd, rspBuff, sizeof(rspBuff));
                }
            }
        }
        else {  /* on other revents, close the socket */
            if (close(pfds[0].fd) == -1){
                handle_error("Close error:");
            }
            num_open_fds--;
        }  

    }
               
}

