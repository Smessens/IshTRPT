#include "packet_implem.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdint.h>
#include <zlib.h>
//#include "packet_interface.h"



struct __attribute__((__packed__)) pkt {
  uint8_t type : 2;
  uint8_t tr : 1;
  uint8_t window : 5;
  uint16_t length : 15;
  uint16_t seqnum : 8;
  uint32_t timestamp : 32;
  uint32_t crc1 : 32;
  uint32_t crc2 : 32;
  char * payload;
};

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

pkt_status_code pkt_decode(const char *data, const size_t len, pkt_t *pkt) {
  uint8_t firstByte;
  memcpy(&firstByte, data, 1); // contient les 8 premiers bits de data : 0 1 type, 2 tr, 3 4 5 6 7 window
  uint8_t WINDOW = (firstByte & (uint8_t)31); // Les 5 derniers bits
  pkt_set_window(pkt, WINDOW);
  uint8_t byteType = firstByte >> 6 ; // Sert juste au décallage
  uint8_t TYPE = (byteType & (uint8_t)3); // les 2 premiers bits
  pkt_set_type(pkt, TYPE);
  uint8_t byteTr = firstByte >> 5 ; // Sert juste au décallage
  uint8_t TR = (byteTr & (uint8_t)1); // Le 3e bit
  pkt_set_tr(pkt, TR);
  uint8_t secondByte;
  memcpy(&secondByte, data+1, 1); // contient les bits 8 à 15 de data
  uint8_t byteL = secondByte >> 7 ; // sert juste au décallage
  uint8_t L = (byteL & 00000001); // le 8e bit
  if (L == 0) { // Length est encodé sur 7 bits
    uint8_t LENGTH = (secondByte & (uint8_t)127); // les bit 9 à 15
    pkt_set_length(pkt, LENGTH);
  }
  else { // Length est encodé sur 15 bits
    uint16_t secNthirdByte;
    memcpy(&secNthirdByte, data+1, 2);
    uint16_t hLENGTH = ntohs(secNthirdByte);
    uint16_t LENGTH = (hLENGTH & (uint16_t)32767); // contient les bits de 9 à 23 ! En network byte order
    pkt_set_length(pkt, LENGTH);
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
//  printf("pkt encode \n" );
  int l=0;
  uint8_t bytebuff;
  uint32_t buff4byte;

  bytebuff= (pkt_get_tr(pkt) << 5) +((pkt_get_type(pkt) << 6) +  pkt_get_window(pkt));
 // printf("bytebuff %d\n",bytebuff);
  memcpy(buf, &bytebuff, 1);
  bytebuff=0;
  uint16_t leng=pkt_get_length(pkt);
  if(leng<=127){
    l=0;
    bytebuff=leng;
    memcpy(buf+1, &bytebuff, 1);
    printf( "buf [1] %d\n",buf[1]); 
  }
  else {
    l = 1;
    uint16_t uleng= htons(leng+32768);
    memcpy(buf+1, &uleng, 2);
  }
//  printf("pkt encode pkt -> length%d\n",pkt->length );

  if ((int)*len<leng+11+l){
    return E_NOMEM;
  }
//  printf("pkt encode post e_nomemi\n" );

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
  if(leng!=0){
      *len= 15+l+leng;
  }
  else{
      *len = 11;
  }
  //printf("window: %d \n ",buf[0]);
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
