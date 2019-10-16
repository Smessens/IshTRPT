#include "packet_interface.h"
#include <stdlib.h>
#include <math.h>#include <string.h>


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

void ctoi(char * data,int * buffer, int  number)
{
  int i;
  char buffData;
  for (i=0;i<16*number;i++){
    if(i%16==0){
      buffData = data[(int)floor(i/16)];
    }
    buffer[i]=(buffData>>1)&1;
  }

}


pkt_t* pkt_new()
{
  pkt_t * new = (pkt_t *) malloc(sizeof(pkt_t));
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

    char * pdata = (char *) malloc(sizeof(char));
    memcpy(pdata,data[0],2);
    ctoi(pdata, buff,1); //take first two bytes and put it in binary in a char table
    free(pdata);
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
    char buffdata2;
    int l=buff[8];                        // finding L
    uint16_t leng;
    if (l==0){
      leng=(uint16_t)btoi(buff,9,15);
    }
    else{
      buffdata2=data[0];
      buffdata=&buffdata2<<8;
      buffdata2=data[1];
      buffdata+=&buffdata2>>8;
      ctoi(buffdata, buff,1);
      leng=(uint16_t)btoi(buff,1,15);
    }
    if(pkt_set_length(pkt,leng)!=PKT_OK){ // erreur si taille ilégale
      return E_LENGTH;
    }

    free(buffdata);

    //set Sequnum
    char * pdata2 =  malloc(sizeof(char));
    pdata2 = &data[1];
    ctoi(data[1],buff,1);
    free(pdata2);
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


    // set timestamp
    char * databuff = (char *)malloc(sizeof(char)*2);
    if (l==0){
      databuff[0]=data[1]<<8;
      databuff[0]+=data[2]>>8;
      databuff[1]=data[2]<<8;
      databuff[1]+=data[3]>>8;
    } else {
      databuff[0] = data[2];
      databuff[1] = data[3];
    }
    ctoi(databuff, buff, 2);
    uint32_t timestamp = (uint32_t) btoi(buff,0,31);
    pkt_set_timestamp(pkt,timestamp);
    free(databuff);

    //set CRC1

    //crc32_z()

    //set payload
    char * payload = (char *)malloc(sizeof(char)*leng);
    int i;

    if (l==0){
      buff[0]=data[5];
      payload[0]=buff[0]<<8;
      for (i=0;i<floor(leng/2);i++){
        buff[0]=data[i+5];
        buff[1]=data[i+6];

        payload[i]=buff[0]<<8;
        payload[i]=payload[i]+(buff[1]>>8);
      }

      if(leng%2==0){
        buff=&data[(int)floor(len/2)];
        buff>>8;
        payload[(int)floor(len/2)]=(buff<<8); // efface le byte non nécessaire
      }
    }

    else{
      for (i=0;i<(int)floor(len/2);i++){
        payload[i]=data[i+6];
      }
      if(leng%2==1){ // si le nombre d'octets est impaire
        buff=&data[(int)floor(len/2)];
        buff>>8;
        payload[(int)floor(leng/2)] =databuff<<8; // efface le byte non nécessaire
      }
    }

    error = pkt_set_payload(pkt,payload,leng);
    if(PKT_OK!=error){
      return E_UNCONSISTENT;
      /*
       /!\ 3 erreur possible a tester !
      */
    }





  free(buff);
  return PKT_OK;
}

pkt_status_code pkt_encode(const pkt_t* pkt, char *buf, size_t *len)
{
  buf[0]=(char) pkt_get_type(pkt);
  buf[1]=(char)pkt_get_tr(pkt);
  buf[2]=(char)pkt_get_window(pkt);
  buf[4]=(char)pkt_get_length(pkt);
  if(buf[4]<64){
    buf[3]=0;
  }
  else{
    buf[3]=1;
  }
  buf[5]=(char)pkt_get_seqnum(pkt);
  buf[6]=(char)pkt_get_timestamp(pkt);
  buf[7]=(char)pkt_get_crc1(pkt);
  buf[8]=(char)pkt_get_payload(pkt);
  buf[9]=(char)pkt_set_crc2(pkt);


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
    return -1;
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
  pkt->crc1 = crc1; // /!\ A modif !!
  return PKT_OK;
}

pkt_status_code pkt_set_crc2(pkt_t *pkt, const uint32_t crc2)
{
  pkt->crc2 = crc2; // /!\ A modif !!
  return PKT_OK;
}

pkt_status_code pkt_set_payload(pkt_t *pkt,
                                const char *data,
                                const uint16_t length){
  if (length>512){
    return E_LENGTH; // juste?
  }
  pkt->payload=data;
  return PKT_OK;                                  // coder les différentes erreurs

}

/*

ssize_t varuint_decode(const uint8_t *data, const size_t len, uint16_t *retval) // coder erreur
{
    return NULL;
}


ssize_t varuint_encode(uint16_t val, uint8_t *data, const size_t len) // coder erreur
{
    return NULL;
}

size_t varuint_len(const uint8_t *data) // coder erreur
{
    return NULL;
}


ssize_t varuint_predict_len(uint16_t val) // coder erreur
{
    return NULL;
}


ssize_t predict_header_length(const pkt_t *pkt)  // coder erreur
{
    int l =pkt_get_length(pkt);
    if(l<64){
      return 11;
    }
    else{
      return 12;
    }
}
*/
