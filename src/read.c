#include "read.h"

const char * real_address(const char *address, struct sockaddr_in6 *rval) {
    printf("real_address\n");
    struct addrinfo new;
    memset(&new,0,sizeof(new));

    new.ai_socktype = SOCK_DGRAM; //socket
    new.ai_family = AF_INET6; // IPv6
    new.ai_protocol = IPPROTO_UDP; //protocol UDP
    struct addrinfo *goal;
    printf("millieu real adresse\n");
    int error;
    error = getaddrinfo(address, NULL, &new, &goal);
    if (error != 0) {
        return gai_strerror(error);
    }
    struct sockaddr_in6 * saddr = (struct sockaddr_in6 *) goal->ai_addr;
    printf("goal->ai_adrr %d, %d\n",goal->ai_addr, saddr);
    printf("sizeof rval %d, %d\n",sizeof(goal->ai_addr),sizeof( saddr));
    printf("millieu 2  real adressse\n");
/*
    int i;
    for (i=0;i<16;i++){
       printf("%d bytes cpy \n",i);
       memcpy((void *)rval,(const void *)saddr,i);// adresse?:wq
    }
*/
    rval=saddr;  
    printf("millieu 3  real adressse\n");
    freeaddrinfo(goal);
    printf("fin real adressse \n");
    return NULL;
}

int create_socket(struct sockaddr_in6 *source_addr, int src_port, struct sockaddr_in6 *dest_addr, int dst_port) {
  printf("create_socket\n");
  int sock = socket(PF_INET6, SOCK_DGRAM, IPPROTO_UDP);
  if (sock == -1) { // error in the creation of the socket
    printf("error 1\n");
    printf("%s\n", strerror(errno));
    return sock;
  }

  int error;
  // binding with source
  if (source_addr != NULL && src_port > 0) {
    printf("source adresse %d \n",source_addr);
    source_addr->sin6_family = AF_INET6;
    source_addr->sin6_port = htons(src_port);
    int yes = 1;
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
	printf("error 2b : \n");
    }
    error = bind(sock,(struct sockaddr *) source_addr, sizeof(struct sockaddr_in6));
    if (error != 0) {
      printf("error 2 : %d\n", error);
      printf("%s\n", strerror(errno));
      return -1;
    }
  }

  // connecting with destination
  if (dest_addr != NULL && dst_port > 0) {
    dest_addr->sin6_family = AF_INET6;
    dest_addr->sin6_port = htons(dst_port);
    error = connect(sock,(struct sockaddr *) dest_addr, sizeof(struct sockaddr_in6));
    if(error != 0) {
      printf("error 3 : %d\n", error);
      printf("%s\n", strerror(errno));
      return -1;
    }
  }
  return sock;
}

int wait_for_client(int sfd) {
  printf("wait_for_client\n");
  char buff[1024];
  struct sockaddr_in6 source_addr;
  socklen_t len = sizeof(struct sockaddr_in6);
  memset(&source_addr, 0, len);
  int err = recvfrom(sfd, buff, (size_t) 1024, MSG_PEEK, (struct sockaddr *) &source_addr, &len);
  if (err == -1) {
    return -1;
  }
  err = connect(sfd, (struct sockaddr *) &source_addr, len);
  printf("wait_for_client fin \n");

  return err;
}
