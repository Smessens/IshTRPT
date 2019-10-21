#include "read.h"

const char * real_address(const char *address, struct sockaddr_in6 *rval) {
    struct addrinfo new;
    memset(&new,0,sizeof(new));

    new.ai_socktype = SOCK_DGRAM; //socket
    new.ai_family = AF_INET6; // IPv6
    new.ai_protocol = IPPROTO_UDP; //protocol UDP
    struct addrinfo *goal;

    int error;
    error = getaddrinfo(address, NULL, &new, &goal);
    if (error != 0) {
        return gai_strerror(error);
    }
    memcpy((void *)rval,(const void *)goal->ai_addr,sizeof(struct sockaddr_in6));
    freeaddrinfo(goal);
    return NULL;
}

int create_socket(struct sockaddr_in6 *source_addr, int src_port, struct sockaddr_in6 *dest_addr, int dst_port) {
  int sock = socket(PF_INET6, SOCK_DGRAM, IPPROTO_UDP);
  if (sock == -1) { // error in the creation of the socket
    printf("error 1\n");
    printf("%s\n", strerror(errno));
    return sock;
  }

  int error;
  // binding with source
  if (source_addr != NULL && src_port > 0) {
    source_addr->sin6_family = AF_INET6;
    source_addr->sin6_port = htons(src_port);
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
  char buff[1024];
  struct sockaddr_in6 source_addr;
  socklen_t len = sizeof(struct sockaddr_in6);
  memset(&source_addr, 0, len);
  int err = recvfrom(sfd, buff, (size_t) 1024, MSG_PEEK, (struct sockaddr *) &source_addr, &len);
  if (err == -1) {
    return -1;
  }
  err = connect(sfd, (struct sockaddr *) &source_addr, len);
  return err;
}
