#include "packet_interface.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdint.h>
#include <zlib.h>





/* Extra #includes */
/* Your code will be inserted here */


struct __attribute__((__packed__)) pkt {
  uint8_t type : 2;
  uint8_t tr : 1;
  uint8_t window : 5;
  uint16_t length : 15;
  uint8_t seqnum : 8;
  uint32_t timestamp : 32;
  uint32_t crc1 : 32;
  uint32_t crc2 : 32;
  char * payload;
}pk_t;

/* Extra code */

// convert a int buffer between begin and end into his int value
int btoi(int* buffer, int begin, int end) {
  int r = buffer[end];
  int i;
  for(i=0; i <= end-begin; i++) {
		r += pow(buffer[end-i]*2,i);
  }
  return r;
}

// convert a char into a int array of 1 and 0
void ctoi(char data,int * buffer)
{
  int i;
  for (i=15;i>=0;i--){
    buffer[i]=data&1;
    data=data>>1;
  }
}


pkt_t* pkt_new()
{
  pkt_t * new = (pkt_t *) malloc(sizeof(pkt_t));
  if(new == NULL)
  {
      return NULL;
  }
  new->type = 0;
  new->tr = 0;
  new->window = 0;
  new->length = 0;
  new->seqnum = 0;
  new->timestamp = 0;
  new->crc1 = 0;
  new->crc2 = 0;
  new->payload = NULL;
  return new;
}

void pkt_del(pkt_t *pkt)
{
    free(pkt->payload);
    free(pkt);
}

pkt_status_code pkt_decode(const char *data, const size_t len, pkt_t *pkt)
{
    int * buff = (int *)malloc(sizeof(int)*32);
    pkt_status_code error;


    ctoi(data[0], buff); //take first two bytes and put it in binary in a char table
    // set type
    ptypes_t pt= (ptypes_t) btoi(buff,0,1);
    if(PKT_OK!=pkt_set_type(pkt,pt)){//check if type is legal
      free(buff);
      return E_TYPE; //send type error
    }
    // set TR
    uint8_t tr = (uint8_t) buff[2];
    if(PKT_OK!=pkt_set_tr(pkt,tr)){//check if tr is ok
      free(buff);
      return E_TR; //send type error
    }

    // set window
    uint8_t window = (uint8_t) btoi(buff,3,7);
    pkt_set_window(pkt,window);

    // set length
    char * buffdata = (char *) malloc(sizeof(char));
    char buffdata2;                        // used if l==1
    int l=buff[8];                        // finding L
    uint16_t leng;
    if (l==0){
      leng=(uint16_t)btoi(buff,9,15);
    }
    else{
      buffdata2=data[0]<<8;
      buffdata2+=data[1]>>8;
      ctoi(buffdata2, buff);
      leng=(uint16_t)btoi(buff,1,15);
    }
    if(pkt_set_length(pkt,leng)!=PKT_OK){ // erreur si taille ilégale
      return E_LENGTH;
    }
    free(buffdata);

    //set Sequnum

    ctoi(data[1],buff);
    uint8_t seqnum;
    if (l==0){
      seqnum=btoi(buff,0,7);
      if(pkt_set_seqnum(pkt,seqnum)!=PKT_OK){ // erreur si taille ilégale
        return E_SEQNUM;
      }
    }
    else{
      seqnum=btoi(buff,8,15);
      if(pkt_set_seqnum(pkt,seqnum)!=PKT_OK){ // erreur si taille ilégale
        return E_SEQNUM;
      }
    }

    uint8_t SEQNUM;
    memcpy(&SEQNUM, data+2+L, 1); // contient les bits 16 à 23
    pkt_set_seqnum(pkt, SEQNUM);
    uint32_t TIMESTAMP;
    memcpy(&TIMESTAMP, data+3+L, 4); // contient les bits 24 à 55
    pkt_set_timestamp(pkt, TIMESTAMP);
    uint32_t CRC1;
    memcpy(&CRC1, data+7+L, 4);
    pkt_set_crc1(pkt, CRC1);
    char* PAYLOAD = (char*)malloc(pkt_get_length(pkt));
    memcpy(PAYLOAD, data+11+L, pkt_get_length(pkt));
    pkt_set_payload(pkt, PAYLOAD, pkt_get_length(pkt));
    uint32_t CRC2;
    memcpy(&CRC2, data+11+L+pkt_get_length(pkt), 4);
    pkt_set_crc2(pkt, CRC2);
    // calcul du crc1 et du crc2
    if(pkt_get_tr(pkt) == 0) {
      uint32_t crc1 = crc32(0L, Z_NULL, 0);
      crc1 = crc32(crc1, ( const Bytef *) data, 7+L);
      if(crc1 != ntohl(pkt_get_crc1(pkt))) {
        return E_CRC;
      }
    }
    if(pkt_get_length(pkt) != 0) {
      uint32_t crc2 = crc32(0L, Z_NULL, 0);
      crc2 = crc32(crc2, (Bytef *)(data+11+L), pkt_get_length(pkt));
      if(crc2 != ntohl(pkt_get_crc2(pkt))) {
        return E_CRC;
      }
    }
    if (pkt_get_length(pkt)+15+L > (int)len) {
      return E_UNCONSISTENT;
    }
    if (pkt_get_length(pkt) > 512) {
      return  E_LENGTH;
    }
    return PKT_OK;
}

pkt_status_code pkt_encode(const pkt_t* pkt, char *buf, size_t *len)
{
  int l=0;
  int L = l;
  uint8_t bytebuff= (pkt_get_tr(pkt) << 5) +((pkt_get_type(pkt) << 6) +  pkt_get_window(pkt));
  memcpy(buf, &bytebuff, 1);
  bytebuff=0;
  uint16_t leng=pkt_get_length(pkt);

  if(leng<=127){
    l=0;
    bytebuff=leng;
    memcpy(buf+1, &bytebuff, 1);
  }
  else {
    l = 1;
    uint16_t uleng= htons(leng+32768);
    memcpy(buf+1, &uleng, 2);
  }

  if ((int)(* len)<(pkt->length)+11+l){
      return E_NOMEM;
  }

  L=l;
  uint8_t seqnumByte = pkt_get_seqnum(pkt);
  memcpy(buf+2+L, &seqnumByte, 1);
  uint32_t timestampByte =  pkt_get_timestamp(pkt);
  memcpy(buf+3+L, &timestampByte, 4);

  if((pkt_get_tr(pkt) == 0)) {
    uint32_t crc1 = crc32(0L, Z_NULL, 0);
    crc1 = crc32(crc1, ( const Bytef *) buf, 7+L);
    uint32_t nCRC1 = htonl(crc1);
    memcpy(buf+7+L, &nCRC1, 4);
  }
  memcpy(buf+11+L, (const void*)pkt_get_payload(pkt), pkt_get_length(pkt));
  if(pkt_get_length(pkt) != 0) {
    uint32_t crc2 = crc32(0L, Z_NULL, 0);
    crc2 = crc32(crc2, (Bytef *)(buf+11+L), pkt_get_length(pkt));
    uint32_t nCRC2 = htonl(crc2);
    memcpy(buf+11+L+pkt_get_length(pkt), &nCRC2, 4);
  }
  *len = (size_t)(11+L+pkt_get_length(pkt)+4);
  return PKT_OK;
}
ptypes_t pkt_get_type  (const pkt_t* pkt)
{
  return pkt->type;
}

uint8_t  pkt_get_tr(const pkt_t* pkt)
{
  return pkt->tr;
}

uint8_t  pkt_get_window(const pkt_t* pkt)
{
  return pkt->window;
}

uint8_t  pkt_get_seqnum(const pkt_t* pkt)
{
  return pkt->seqnum;

}

uint16_t pkt_get_length(const pkt_t* pkt)
{
  return pkt->length;

}

uint32_t pkt_get_timestamp   (const pkt_t* pkt)
{
  return pkt->timestamp;
}

uint32_t pkt_get_crc1   (const pkt_t* pkt)
{
  return pkt->crc1;
}

uint32_t pkt_get_crc2   (const pkt_t* pkt)
{
  return pkt->crc2;
}

const char* pkt_get_payload(const pkt_t* pkt)
{
  return pkt->payload;
}


pkt_status_code pkt_set_type(pkt_t *pkt, const ptypes_t type)
{
  if (type==4){ // illegal
    return -1;
  }
  if(type==1){
    pkt->type=PTYPE_DATA;
  }
  if(type==2){
    pkt->type=PTYPE_ACK;
  }
  if(type==3){
    pkt->type=PTYPE_NACK;
  }
  return PKT_OK;
}

pkt_status_code pkt_set_tr(pkt_t *pkt, const uint8_t tr)
{
  if (pkt_get_type(pkt) != PTYPE_DATA && tr != 0) {
    return E_TR;
  }
  pkt->tr = tr;
  return PKT_OK;
}

pkt_status_code pkt_set_window(pkt_t *pkt, const uint8_t window)
{
  pkt->window = window;
  return PKT_OK;
}

pkt_status_code pkt_set_seqnum(pkt_t *pkt, const uint8_t seqnum) // /!\ sequnum erreur a coder
{
  pkt->seqnum = seqnum;
  return PKT_OK;
}

pkt_status_code pkt_set_length(pkt_t *pkt, const uint16_t length)
{
  if (length>512){
    return E_LENGTH;
  }
  pkt->length=length;
  return PKT_OK;
}

pkt_status_code pkt_set_timestamp(pkt_t *pkt, const uint32_t timestamp)
{
  pkt->timestamp = timestamp;
  return PKT_OK;
}

pkt_status_code pkt_set_crc1(pkt_t *pkt, const uint32_t crc1)
{
  pkt->crc1 = crc1;
  return PKT_OK;
}

pkt_status_code pkt_set_crc2(pkt_t *pkt, const uint32_t crc2)
{
  pkt->crc2 = crc2;
  return PKT_OK;
}

pkt_status_code pkt_set_payload(pkt_t *pkt,
                                const char *data,
                                const uint16_t length){

  pkt->payload = (char *)malloc(sizeof(char)*length);

  if(pkt->payload == NULL){return E_NOMEM;}

  memcpy(pkt->payload,data,length);
  pkt_set_length(pkt,length);
  return PKT_OK;  // coder les différentes erreurs
}



ssize_t varuint_decode(const uint8_t *data, const size_t len, uint16_t *retval)
{
if(!data[0]){ // si le premier bit vaut 0
        if(len < 7){ return -1;}
        memcpy(retval,data+1,7);
        return 1;
    }
    else{ // si le premier bit vaut 1
        if(len < 15){ return -1;}
        memcpy(retval,data+1,15);
        return 2;
    }
}


ssize_t varuint_encode(uint16_t val, uint8_t *data, const size_t len)
{
if((sizeof(val)+1) > len){ //si val plus le bit d indication est plus grand que la taille de data
    return -1;
}
if(sizeof(val)==7){ //si val vaut 7 alors premier bit mis à 0
    data[0] = 0;
    memcpy(data+1,&val,7);
    return 1;

}

if(sizeof(val)==15){
    data[0] = 1;
    memcpy(data+1,&val,15);
    return 1;
}

return -1;
}

size_t varuint_len(const uint8_t *data)
{
    return (sizeof(*data)/8);
}


ssize_t varuint_predict_len(uint16_t val)
{
    if (val >= 0x8000)
    {
        return -1;
    }
    return (sizeof(val)/8);
}


ssize_t predict_header_length(const pkt_t *pkt)
{
    if (pkt->length >= 0x8000)
    {
        return -1;
    }
    return 2;
}
