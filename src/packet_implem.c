#include "packet_implem.h"
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
  uint8_t buffbyte;
  uint16_t buff2byte;
  uint32_t buff4byte;
  int leng;


  int * buff = (int *)malloc(sizeof(int)*32);
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
  memcpy(&buffbyte, data, 1);
  buffbyte= buffbyte<<3;
  buffbyte= buffbyte>>3;
  pkt_set_window(pkt, buffbyte);

  // set length
  uint8_t l=buff[8];                        // finding L
  memcpy(&buffbyte, data+1, 1);
  buffbyte= buffbyte>> 7 ;

  if (l == 0) { // petite lenght
    memcpy(&buffbyte, data+1, 1);
    pkt_set_length(pkt,buffbyte);
  }
  else { // Length est encodé sur 15 bits
    memcpy(&buffbyte, data+1, 1);
    buffbyte-=-128; // enlève le l
    buff2byte=buffbyte;
    buff2byte=buff2byte<<8;
    memcpy(&buffbyte, data+2, 1);
    buff2byte+=buffbyte;
    pkt_set_length(pkt,buff2byte);
  }
  leng=pkt_get_length(pkt);

  if (leng+15+l > (int)len) {
    return E_UNCONSISTENT;
  }
  if (leng> 512) {
    return  E_LENGTH;
  }


  //set seqnum
  memcpy(&buffbyte, data+2+l, 1);
  pkt_set_seqnum(pkt,buffbyte);
  //set timestamp
  memcpy(&buff4byte, data+3+l, 4);
  pkt_set_timestamp(pkt,buff4byte);

  memcpy(&buff4byte, data+7+l, 4);  // get crc1
  pkt_set_crc1(pkt,buff4byte);

  char*  payload= (char*)malloc(leng);

  memcpy(payload, data+11+l, leng);
  pkt_set_payload(pkt,payload,leng);

  memcpy(&buff4byte, data+11+l+leng, 4); //get crc2
  pkt_set_crc2(pkt,buff4byte);

  if(pkt_get_tr(pkt) == 0) {
    buff4byte = crc32(0L, Z_NULL, 0);
    buff4byte  = crc32(buff4byte , ( const Bytef *) data, 7+l);

    if(buff4byte  != ntohl(pkt_get_crc1(pkt))) {
      return E_CRC;
    }
  }

  if(leng!= 0) {

    buff4byte= crc32(0L, Z_NULL, 0);
    buff4byte = crc32(buff4byte, (Bytef *)(data+11+l),leng);
    if(buff4byte != ntohl(pkt_get_crc2(pkt))) {
      return E_CRC;
    }
  }



  return PKT_OK;
}

pkt_status_code pkt_encode(const pkt_t* pkt, char *buf, size_t *len)
{
  int l=0;
  uint8_t bytebuff;
  uint32_t buff4byte;

  bytebuff= (pkt_get_tr(pkt) << 5) +((pkt_get_type(pkt) << 6) +  pkt_get_window(pkt));
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

  bytebuff= pkt_get_seqnum(pkt);
  memcpy(buf+2+l, &bytebuff, 1);

  buff4byte =  pkt_get_timestamp(pkt);
  memcpy(buf+3+l, &buff4byte, 4);

  if((pkt_get_tr(pkt) == 0)) { //crc1
    buff4byte = crc32(0L, Z_NULL, 0);
    buff4byte = crc32(buff4byte, ( const Bytef *) buf, 7+l);
    buff4byte= htonl(buff4byte);
    memcpy(buf+7+l, &buff4byte, 4);
  }

  memcpy(buf+11+l, (const void*)pkt_get_payload(pkt), leng);
  if(leng != 0) {  //crc2 //ish
    buff4byte= crc32(0L, Z_NULL, 0);
    buff4byte= crc32(buff4byte, (Bytef *)(buf+11+l),leng);
    buff4byte= htonl(buff4byte);
    memcpy(buf+11+l+leng, &buff4byte, 4);
  }

  *len = (size_t)(15+l+leng);
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

  if(type==1){
    pkt->type=PTYPE_DATA;
  }
  if(type==2){
    pkt->type=PTYPE_ACK;
  }
  if(type==3){
    pkt->type=PTYPE_NACK;
  }
  if (type==0){ // illegal
    return -1;
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
