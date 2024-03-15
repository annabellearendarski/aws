#include <errno.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>


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
    int current_size;
    char *rspBuff="ServerNotImplemented";
    char readBuf[4];
    bool conn_to_close;

  
    servfd = socket(AF_INET, SOCK_STREAM, 0);

    if (servfd == -1){
        perror("socket creation error:"); 
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
    ret=bind(servfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr));
    
    if (ret==-1){
        perror("Bind error:");
        exit(EXIT_FAILURE);
    }

    ret=listen(servfd, SERVER_BACKLOG_SIZE);
    
    if (ret == -1){
        perror("Listen error:");
        exit(EXIT_FAILURE);
    }
    
    pfds[0].fd=servfd;
    pfds[0].events = POLLIN;
    
    while(1){
    
        ret=poll(pfds, num_open_fds, 1);
        
        if (ret == -1){
            perror("Poll error:");
            exit(EXIT_FAILURE);
        }

        current_size=num_open_fds;
        
        for(int i=0;i<current_size;i++) {
            conn_to_close= false;

            if (pfds[i].revents==0) {
                continue;
            }

            if(pfds[i].revents == POLLIN) {

                if (pfds[i].fd == servfd ){

                    if(num_open_fds == MAX_NB_FD){
                        printf("Cannot accept more socket\n Socket sent is ignored");
                        continue;
                    }

                    client_addr_size = sizeof(client_addr);
                    clientfd = accept(servfd, (struct sockaddr *) &client_addr, &client_addr_size);
                    pfds[num_open_fds].fd = clientfd;
                    pfds[num_open_fds].events = POLLIN;
                    num_open_fds = num_open_fds + 1;
                    
                }
                else{
                    ret = recv(pfds[i].fd, readBuf, sizeof(readBuf),0);
                    
                    if (ret == -1){
                        perror("Read error:");
                        conn_to_close=true;
                    }
                    else if (ret == 0){
                        conn_to_close=true;
                    }
                    else{
                        send(pfds[i].fd, rspBuff, strlen(rspBuff)+1,0);
                    }

                }
            }
            else{
                conn_to_close=true;
            }

            if (conn_to_close == true){
                pfds[i].fd=-1;
            }

        }

        for (int i = 0; i < num_open_fds; i++){
            if (pfds[i].fd == -1){
                for(int j = i; j < num_open_fds-1; j++){
                    pfds[j].fd = pfds[j+1].fd;
                }
                i--;
                num_open_fds--;
            }
        }
    }
               
}

