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

int read_sock(const int sfd, char * buffer, FILE * log) {
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
    return read(sfd, buffer, 528);
  }
  if(err == -1) {
    fprintf(log, "issue : %s (read_sock)\n",strerror(errno));
  }
  else if(err == 0) {
    fprintf(log,"nothing read on the socket with %d seconds\n",TV);
  }
  return err;
}

//fonction permettant d'envoyer des ack
int send_ack(int sock,uint8_t seqnum,uint32_t window, uint8_t tr,uint32_t timestamp, FILE * log){
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
  pkt_status_code error = pkt_encode((const pkt_t *)pktack,buff,len);
  if(error != PKT_OK){
    fprintf(log, "PKT error with number : %d (send_ack)\n",error); // a completer
    pkt_del(pktack);
    return -1;
  }

  else {
    send(sock,buff,*len,0);
  }
  pkt_del(pktack);
  return 0;
}


int selective(int socket,int filename, FILE * log){
  pkt_t * databuff [32];// 32=MAX_WINDOW_SIZE

  uint32_t window = 31;
  int i;
  for (i = 0; i < 31; i++) {
    databuff[i]=NULL;
  }
  uint8_t expected_seqnum = 0;
  uint32_t last_time=0;
  char data[528];

  int error=0;
  pkt_t * new_pkt = (pkt_t *) malloc(sizeof(struct pkt_t*));
  int place;
  bool disconnect = false;
  fclose(log);
  while(!disconnect){
    log = fopen("log.txt","a");
    memset((void *)data, 0, 528);
    error = read_sock(socket, data,log);
    if (error < 0) {
      fprintf(log, "issue with read_sock (selective)\n");
      error=0;
    }
    else if(error==0){
      fprintf(log,"rien lu\n");
    }
    else{
      if(data ==NULL){
        fprintf(log,"data null \n");
      }
      pkt_status_code e = pkt_decode(data,error,new_pkt);
      if(e != PKT_OK) {  // pkt est invalide , abandon
        pkt_del(new_pkt);
        fprintf(log,"issue with pkt_decode (selective)%d\n",e);
        error=0;
      }
      else{// le pkt est valide , vérification de seqnum
        if(pkt_get_type(new_pkt) != PTYPE_DATA) {
          pkt_del(new_pkt);
        }
        //check of disconnection
        else if(pkt_get_length(new_pkt)==0 && pkt_get_seqnum(new_pkt) == expected_seqnum) {
          send_ack(socket,expected_seqnum+1,window,0,last_time,log);
          disconnect = true;
        }
        else{
          if (pkt_get_seqnum(new_pkt) == expected_seqnum) { //c'est le seqnum qu'on attendait
            uint8_t last_tr = 0;
            write(filename,pkt_get_payload(new_pkt),pkt_get_length(new_pkt));
            printf("ecriture du paquet : %d\n",pkt_get_seqnum(new_pkt));
	    expected_seqnum=expected_seqnum+1;
            last_time=pkt_get_timestamp(new_pkt);
            last_tr=pkt_get_tr(new_pkt);
            pkt_del(new_pkt);
            bool isnotlast=true;
            while(isnotlast) { // vérification dans le buffer si le seqnum suivant est présent
              isnotlast=false;
              int j;
              for(j = 0; j < 31; j++) {
                if(databuff[j]!=NULL && !isnotlast) {
                  if(pkt_get_seqnum(databuff[j])==expected_seqnum) {
                    write(filename,pkt_get_payload(databuff[j]),pkt_get_length(databuff[j]));
                    expected_seqnum = expected_seqnum+1;
                    last_time=pkt_get_timestamp(databuff[j]);
                    last_tr=pkt_get_tr(databuff[j]);
                    pkt_del(databuff[j]);
                    databuff[j]=NULL;
                  }
                }
              }
            }
            send_ack(socket,expected_seqnum,window,last_tr,last_time,log);
          }
          // le paquet est en desordre
          else if((pkt_get_seqnum(new_pkt)>expected_seqnum && pkt_get_seqnum(new_pkt)<expected_seqnum+window) ||
          (pkt_get_seqnum(new_pkt)>0 && pkt_get_seqnum(new_pkt)<(expected_seqnum+window)%256 && expected_seqnum+window>255)) {
            place = -1;
            bool flag=true;
            for (i= 0; i < 31; i++) {
              if(databuff[i] == NULL) {
                place = i;
              }
              else if (pkt_get_seqnum(databuff[i])==pkt_get_seqnum(new_pkt)){
                pkt_del(new_pkt);
                flag=false;
                break;
              }
            }
            if(flag){
              if(place!=-1){
                databuff[place] = new_pkt;
                window--;
              }
            }

            send_ack(socket,expected_seqnum,window,0,last_time,log);
          } // paquet pas dans window
          else {
            send_ack(socket,expected_seqnum,window,0,last_time,log);
            pkt_del(new_pkt);
            fprintf(log, "paquet pas dans window (selective)\n");
          }
        }
      }
    }
    fclose(log);
  }

  close(socket);
  close(filename);

  return 0;
}
