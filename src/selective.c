#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h> /* * sockaddr_in6 */
#include <sys/types.h> /* sockaddr_in6 */
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <errno.h>
#include <stdbool.h>

#include "read.h"
#include "packet_implem.h"
#include "selective.h"

#define STDOUT 1
#define TV 5

int read_sock(const int sfd, char * buffer) {
  int max_sfd = sfd+1;
  fd_set fd_read;
  int err;
  struct timeval tv;
  tv.tv_sec = TV;
  tv.tv_usec = 0;

  FD_ZERO(&fd_read);
  FD_SET(sfd,&fd_read);
  err = select(max_sfd, &fd_read, NULL, NULL, &tv);
  if (FD_ISSET(sfd,&fd_read)) {
    printf("FD_isset\n");
    return read(sfd, buffer, 528);
  }
  if(err == -1) {
    printf("%s\n",strerror(errno));
  }
  else if(err == 0) {
    printf("nothing read on the socket avec the %d seconds\n",TV);
  }
  return err;
}

int send_ack(int sock,uint8_t seqnum,uint32_t window, uint8_t tr,uint32_t timestamp ){
  printf("Sended ack with sequnum %d   window : %d\n",seqnum,window);
  pkt_t * pktack = pkt_new();
  pkt_set_seqnum(pktack,seqnum);
  pkt_set_window(pktack,window);
  pkt_set_timestamp(pktack,timestamp);
  if(tr==1) {
    pkt_set_type(pktack,PTYPE_NACK);
  }
  else {
    pkt_set_type(pktack,PTYPE_ACK);
  }
  size_t var =12;
  size_t *  len = &var; 
  char buff[12];
//  printf("ack pre-encode\n");
  pkt_status_code error = pkt_encode((const pkt_t *)pktack,buff,len);
//  printf("ack posst-encode\n");
  if(error != PKT_OK){
    fprintf(stderr, "PKT error %d\n",error); // a completer
    pkt_del(pktack);
    return -1;
  }
  
  else {
    
    send(sock,buff,*len,0);
  }
  pkt_del(pktack);
  return 0;
}


int selective(int socket,int filename){
  pkt_t * databuff [32];// 32=MAX_WINDOW_SIZE
  printf("selective start \n");
  uint32_t window = 31;
  int i;
  for (i = 0; i < 32; i++) {
    databuff[i]=NULL;
  }
  uint8_t expected_seqnum = 0;
  uint32_t last_time;
  char data[528];

  int error=0;
  pkt_t * new_pkt = (pkt_t *) malloc(sizeof(struct pkt_t*));
  int place;
  bool disconnect = false;
  while(!disconnect){
    memset((void *)data, 0, 528); //524 ou 272 ???
//    printf("while!disconnect\n");
    error = read_sock(socket, data);
//    printf("data[0] %d\n",data[0]);
    if (error < 0) {
      fprintf(stderr, "issue with read_sock\n");
      error=0;

    }
    else if(error==0){
      printf("rien lu\n");
    }
    else{
    if(data ==NULL){
      printf("data null \n");
    }
    pkt_status_code e = pkt_decode(data,error,new_pkt);
    if(e != PKT_OK) {
      printf("error pkt = %d\n",e);
      fprintf(stderr,"issue with pkt \n");
      error=0;
    }
    printf("pkt recu length :%d  seqnum : %d\n",pkt_get_length(new_pkt),pkt_get_seqnum(new_pkt));
    if(pkt_get_type(new_pkt) != PTYPE_DATA) {
      free(new_pkt);
    }
    //printf("pkt recu length %d\n",pkt_get_length(new_pkt));
    //check of disconnection
    else if(pkt_get_length(new_pkt)==0 && pkt_get_seqnum(new_pkt) == expected_seqnum-1) {
       printf("disconnect = true\n");
       disconnect = true;
    }
    else{
      if(pkt_get_seqnum(new_pkt) == expected_seqnum) { // le paquet attendu
        printf("packet est dans l'odre \n");
        bool isnotlast=true;
        while(isnotlast){
          write(filename,pkt_get_payload(new_pkt),pkt_get_length(new_pkt)); ///
//          printf("just writed \n");
          isnotlast=false;
          expected_seqnum=expected_seqnum+1;
          last_time=pkt_get_timestamp(new_pkt);

          for (i = 0; i < 32; i++){
            if(databuff[i]!=NULL&&!isnotlast){
              if(expected_seqnum==pkt_get_seqnum(databuff[i])){
                new_pkt=databuff[i];
                databuff[i]=NULL;
                window--;
                isnotlast =true;
              }
            }
          }
        }
      //    printf("fin while is not last \n");
        printf("window : %d\n",window);
        send_ack(socket,expected_seqnum,window,pkt_get_tr(new_pkt),last_time);
      //  printf("post send ack\n");
        }

      // le paquet est en desordre
      else if((pkt_get_seqnum(new_pkt)>expected_seqnum && pkt_get_seqnum(new_pkt)<expected_seqnum+window) ||
          (pkt_get_seqnum(new_pkt)>0 && pkt_get_seqnum(new_pkt)<(expected_seqnum+window)%256 && expected_seqnum+window>255)) {
        place = -1;
        printf("packet en désordre \n");
        for (i= 0; i < 32; i++) {
          if(databuff[i] == NULL) {
            place = i;
          }
          else if (pkt_get_seqnum(databuff[i])==pkt_get_seqnum(new_pkt)){
            break;
          }
          if(i==31){
            if(place!=-1){
              databuff[place]=new_pkt;
              window++;
            }
          }
        }
        send_ack(socket,expected_seqnum,window,0,last_time);
      } // paquet pas dans window
      else {
        printf("pkt seqnum : %d   expected seqnum %d\n",pkt_get_seqnum(new_pkt),expected_seqnum);
        send_ack(socket,expected_seqnum,window,0,last_time);
        fprintf(stderr, "paquet pas dans window\n");
      }
    }}
  }
  close(socket);
  close(filename);
  free(new_pkt);
  printf("fin de selective\n");
  return 0;
}
