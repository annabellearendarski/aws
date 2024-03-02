#ifndef CSUTILS_H
#define CSUTILS_H

/*
    create http socket
    return a file descriptor, -1 if error
*/
static int csutils_socket_http_build();

/*
    create socket address object
    return a pointer on a sockaddr_in struct
*/
static void csutils_sock_addr_in_create(sockaddr_in *sockAddr,int *addr,int port);

/*
    create client server chat
*/
static void csutils_client_server_chat_create(char *buf);
  

#endif /* CSUTILS_H */