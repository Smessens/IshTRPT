#ifndef __SELECTIVE_H_
#define __SELECTIVE_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h> /* * sockaddr_in6 */
#include <sys/types.h> /* sockaddr_in6 */
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <errno.h>


int read_sock(const int sfd, char * buffer);

int send_ack(int sock,uint8_t seqnum,uint32_t window, uint8_t tr);

int selective(int socket,int fd);


#endif
