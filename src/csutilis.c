#include "csutilis.h"
#include <sys/socket.h>
#include <arpa/inet.h>

int
csutils_socket_http_build(){
    return socket(AF_NET, SOCK_STREAM, 0);
}

void 
static void csutils_sock_addr_in_create(sockaddr_in *sockAddr,int *addr,int port){
   
    servaddr->sin_family = AF_INET;
    servaddr->sin_addr.s_addr = inet_addr(*addr);
    servaddr->sin_port = htons(port);
    /*/ assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(PORT); 

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);*/

}