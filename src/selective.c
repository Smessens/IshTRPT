#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h> /* * sockaddr_in6 */
#include <sys/types.h> /* sockaddr_in6 */
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <errno.h>

#define STDOUT 1
#define TV 5


int send_ack(int sock,uint8_t seqnum,uint32_t window, uint8_t tr){
  pkt* pktack=pkt_new();
  pkt_set_seqnum(pktack,seqnum);
  pkt_set_window(pktack,window);
  pkt_set_length(pktack,0);
  if(tr==1) {
    pkt_set_type(pktack,PTYPE_NACK);
  }
  else {
    pkt_set_type(pktack,PTYPE_ACK);
  }
  size_t *len;
  char buff[12];
  pkt_status_code error = pkt_encode(pkt,buff,len)
  if(error != PKT_OK){
    fprintf(stderr, "PKT error %s\n"); // a completer
    pkt_del(pktack);
    return -1;
  }
  else {
    send(sock,buff,sizeof(buff),0);
  }
  pkt_del(pktack);
  return 0;
}

int selective(int socket){
  pkt_t * databuff [32];// 32=MAX_WINDOW_SIZE
  for (i = 0; i < 32; i++) {
    databuff[i]=NULL;
  }
  uint8_t expected_seqnum = 0;
  char buffer[524];
  memset((void *)data, 0, 276);
  int err;
  pkt_t * new_pkt = (pkt_t *) malloc(sizeof(struct pkt_t));
  while(/** TODO **/){
    error = read_sock(socket, data);
    if (error < 0) {
      fprintf(stderr, "issue with read_sock\n");
    }
    pkt_status_code e = pkt_decode(data,error,new_pkt);
    if(e != PKT_OK) {
      fprintf(stderr, e);
    }
    if(pkt_get_type(new_pkt) != PTYPE_DATA) {
      return -1;
    }
    if(pkt_get_seqnum(new_pkt) == expected_seqnum) { // le paquet attendu
      for (int i = 0; i < 32 && databuff[i]!=NULL; i++) { // ignore a 32
        /* code */
      }
      // TODO insert dans buffer etc
      send_ack(socket,expected_seqnum,pkt_get_window(new_pkt),pkt_get_tr(new_pkt));
      expected_seqnum++;
    }
    else if(pkt_get_seqnum(new_pkt) > expected_seqnum) { // le paquet en desordre

    }
    else(pkt_get_seqnum(new_pkt) < expected_seqnum){


    }

  }
  free(new_pkt);
}

int read_sock(const int sfd, char * buffer) {
  int max_sfd = sfd+1;
  fd_set fd_read;

  struct timeval tv;
  tv.tv_sec = TV;
  tv.tv_usec = 0;

  FD_ZERO(&fd_read);
  FD_SET(sfd,&fd_read);
  select(max_sfd, &fd_read, NULL, NULL, &tv);

  if (FD_ISSET(sfd,&fd_read)) {
    return read(sfd, buffer, 524);
  }
  close(sfd);
}
