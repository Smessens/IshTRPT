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
    printf("nothing read on the socket avec the %d seconds\n",TV);
  }
  return err;
}

int send_ack(int sock,uint8_t seqnum,uint32_t window, uint8_t tr,uint32_t timestamp, FILE * log){
  printf("Sended ack with sequnum %d   window : %d  and timestamp%d\n",seqnum,window,timestamp);
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
  // printf("ack pre-encode\n");
  pkt_status_code error = pkt_encode((const pkt_t *)pktack,buff,len);
  //  printf("ack posst-encode\n");
  if(error != PKT_OK){
    fprintf(log, "PKT error with number : %d (send_ack)\n",error); // a completer
    pkt_del(pktack);
    return -1;
  }

  else {

    send(sock,buff,*len,0);
  }
  pkt_del(pktack);
  printf("ack ok \n");
  return 0;
}


int selective(int socket,int filename, FILE * log){
  pkt_t * databuff [32];// 32=MAX_WINDOW_SIZE

  uint32_t window = 31;
  int i;
  for (i = 0; i < 31; i++) {
    //    databuff[i]=(pkt_t *) malloc(sizeof(struct pkt_t*));
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
    log = fopen("log.txt","a");  //set to *log
    memset((void *)data, 0, 528); //524 ou 272 ???
    printf("while!disconnect\n");
    error = read_sock(socket, data,log);
    //    printf("data[0] %d\n",data[0]);
    if (error < 0) {
      fprintf(log, "issue with read_sock (selective)\n");
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
        pkt_del(new_pkt);
        fprintf(log,"issue with pkt_decode (selective)%d\n",e);
        error=0; // est ce qu'on enverait un acjk pour jump start
      }
      else{
        printf("pkt recu length :%d  seqnum : %d\n",pkt_get_length(new_pkt),pkt_get_seqnum(new_pkt));
        if(pkt_get_type(new_pkt) != PTYPE_DATA) {
          pkt_del(new_pkt);
        }
        //printf("pkt recu length %d\n",pkt_get_length(new_pkt));
        //check of disconnection
        else if(pkt_get_length(new_pkt)==0 && pkt_get_seqnum(new_pkt) == expected_seqnum) {
          send_ack(socket,expected_seqnum+1,window,0,last_time,log);
          printf("disconnect = true\n");
          pkt_del(new_pkt);
          disconnect = true;
        }
        else{
          if (pkt_get_seqnum(new_pkt) == expected_seqnum) {
            uint8_t last_tr = 0;
            printf("packet est dans l'order\n");
            write(filename,pkt_get_payload(new_pkt),pkt_get_length(new_pkt));
            expected_seqnum=expected_seqnum+1;
            last_time=pkt_get_timestamp(new_pkt);
            last_tr=pkt_get_tr(new_pkt);
            pkt_del(new_pkt);
            bool isnotlast=true;
            while(isnotlast) {
              isnotlast=false;
              int j;
              for(j = 0; j < 31; j++) {
                if(databuff[j]!=NULL && !isnotlast) {
                  if(pkt_get_seqnum(databuff[j])==expected_seqnum) {
                    printf("Trouvé un autre paquet en %d\n",j);
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
            printf("packet en désordre \n");
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
            printf("pkt seqnum : %d   expected seqnum %d\n",pkt_get_seqnum(new_pkt),expected_seqnum);
            send_ack(socket,expected_seqnum,window,0,last_time,log);
            pkt_del(new_pkt);
            fprintf(log, "paquet pas dans window (selective)\n");
          }
        }
      }
    }
    fclose(log);
  }
  for(i=0;i<32;i++){
	 if(databuff[i]!=NULL){
	   pkt_del(databuff[i]);
	 }
  }
  close(socket);
  close(filename);

  printf("fin de selective\n");
  return 0;
}
