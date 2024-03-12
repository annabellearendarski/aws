#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>

#include <unistd.h>
#include <poll.h>

/*
* tcp server implementation
*/

#define PORT                1026
#define SERVER_BACKLOG_SIZE 2
#define MAX_NB_FD           10


int
main(void)
{   
    int servfd;
    int clientfd;
    int ret;
    struct sockaddr_in serv_addr;
    struct sockaddr_in client_addr;
    socklen_t  client_addr_size;
    struct pollfd  pfds[MAX_NB_FD];
    int num_open_fds=1;
    char *rspBuff="ServerNotImplemented";

    char *readBuf[4];

  
    servfd = socket(AF_INET, SOCK_STREAM, 0);

    if (servfd == -1){
        perror("socket creation error:"); 
        exit(EXIT_FAILURE);
    }
    printf("Socket\n");


    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
    ret=bind(servfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr));
    
    if (ret==-1){
        perror("Bind error:");
        exit(EXIT_FAILURE);
    }
    printf("preListen\n");
    ret=listen(servfd, SERVER_BACKLOG_SIZE);
    printf("postisten\n");
    
    if (ret == -1){
        perror("Listen error:");
        exit(EXIT_FAILURE);
    }
    
    pfds[0].fd=servfd;
    pfds[0].events = POLLIN;
    
    while(1){
    
        ret=poll(pfds, num_open_fds, -1);

        if (ret == -1){
            perror("Poll error:");
            exit(EXIT_FAILURE);
        }

        for(int i=0;i<num_open_fds;i++) {
            printf("tracking %d\n",i);

            if (!pfds[i].revents) {
                continue;
            }

            if(pfds[i].revents == POLLIN) {

                if (i == 0){
                    printf("polling on id0");
                    if(num_open_fds == MAX_NB_FD){
                        printf("Cannot accept more socket\n Socket sent is ignored");
                        continue;
                    }

                    client_addr_size = sizeof(client_addr);
                    clientfd = accept(pfds[0].fd, (struct sockaddr *) &client_addr, &client_addr_size);
                    num_open_fds = num_open_fds + 1;
                    pfds[num_open_fds].fd = clientfd;
                    pfds[num_open_fds].events = POLLIN;
                    


                }
                else{

                    ret = read(pfds[i].fd, readBuf, sizeof(readBuf));

                    if (ret == -1){
                        perror("Read error:");
                        exit(EXIT_FAILURE);
                    }
                    else{
                        write(clientfd, rspBuff, sizeof(rspBuff));
                    }

                }
            } 
        }
    }
               
}

