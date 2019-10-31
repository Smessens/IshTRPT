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

//prend une socket déja ouverte et écris son contenu dans un buffer, tout erreur est
int read_sock(const int sfd, char * buffer, FILE * log);

int send_ack(int sock,uint8_t seqnum,uint32_t window, uint8_t tr,uint32_t timestamp, FILE * log);

int selective(int socket,int fd, FILE * log);


#endif
