#include "packet_interface.h"

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
/* Your code will be inserted here */

pkt_t* pkt_new()
{
  pkt_t new = (pkt_t *) malloc(sizeof(pkt_t));
  new->type = 0;
  new->tr = 0;
  new->window = 0;
  new->length = 0;
  new->seqnum = 0;
  new->timestamp = 0;
  new->crc1 = 0;
  new->crc2 = 0;
  new->payload = null;

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
    if(PKT_OK!=pkt_set_tr(pkt,tr);){//check if tr is ok
      free(buff);
      return E_TR; //send type error
    }

    // set window
    uint8_t window = (uint8_t) btoi(buff,3,7);
    pkt_set_window(pkt,window);

    // set length
    int l=buff[8];
    uint16_t len;
    if (l==0){
      len==btoi(buff,9,15);
    }
    else{
      char buffdata=data[0]<<8;
      buffdata+=data[1]>>8;
      ctoi(buffdata, buff)
      len==btoi(buff,1,15);
    }
    if(pkt_set_length(pkt,len)!=PKT_OK){ // erreur si taille ilégale
      return E_LENGTH
    }

    ctoi(data[1],buff);
    //set Sequnum
    uint8_t seqnum;
    if (l==0){
      seqnum=btoi(buff,0,7);
      if(pkt_set_seqnum(pkt,sequnum)!=PKT_OK){ // erreur si taille ilégale
        return E_SEQNUM;
      }
    }
    else{
      seqnum=btoi(buff,8,15);
      if(pkt_set_seqnum(pkt,sequnum)!=PKT_OK){ // erreur si taille ilégale
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
    ctoi(databuff, buff);
    uint32_t timestamp = (uint32_t) btoi(buff,0,31);
    pkt_set_timestamp(pkt,timestamp);
    free(databuff);

    //set CRC1
    //crc32_z()

    //set payload

    char * payload = (char *)malloc(sizeof(char)*len);
    int i;
    if (l==0){
      data[5]=databuff[0];
      payload[0]=databuff[0]<<8;
      for (i=0;i<floor(len/2);i++){
        databuff[0]=data[i+5];
        databuff[1]=data[i+6];

        payload[i]=databuff[0]<<8;
        payload[i]=payload[i]+databuff[1]>>8;
      }

      if(len%2=0){
        databuff=data[floor(len/2)]>>8;
        payload[floor(len/2)] =databuff<<8; // efface le byte non nécessaire
      }
    }

    else{
      for (i=0;i<floor(len/2);i++){
        payload[i]=data[i+6];
      }
      if(len%2=1){ // si le nombre d'octets est impaire
        databuff=data[floor(len/2)]>>8;
        payload[floor(len/2)] =databuff<<8; // efface le byte non nécessaire
      }
    }

    error
    if(PKT_OK!=pkt_set_payload(pkt,payload,len)){
      return
    }





  free(buff);
  return PKT_OK;
}

pkt_status_code pkt_encode(const pkt_t* pkt, char *buf, size_t *len)
{
    /* Your code will be inserted here */
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
  pkt->sequnum = sequnum;
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
    /* Your code will be inserted here */
}

pkt_status_code pkt_set_crc2(pkt_t *pkt, const uint32_t crc2)
{
    /* Your code will be inserted here */
}

pkt_status_code pkt_set_payload(pkt_t *pkt,
                                const char *data,
                                const uint16_t length){

}


ssize_t varuint_decode(const uint8_t *data, const size_t len, uint16_t *retval)
{
    /* Your code will be inserted here */
}


ssize_t varuint_encode(uint16_t val, uint8_t *data, const size_t len)
{
    /* Your code will be inserted here */
}

size_t varuint_len(const uint8_t *data)
{
    /* Your code will be inserted here */
}


ssize_t varuint_predict_len(uint16_t val)
{
    /* Your code will be inserted here */
}


ssize_t predict_header_length(const pkt_t *pkt)
{
    /* Your code will be inserted here */
}

// convert a int buffer between begin and end into his int value
int btoi(int* buffer, int begin, int end) {
	int r = buffer[end];
	for(int i = 1; i <= end-begin; i++) {
		r += pow(buffer[end-i]*2,i);
	}
  return r;
}

void ctoi(char [] data,int * buffer, int  number)
{
  int i;
  for (i=0;i<16*number;i++){
    buff[i]=data[floor(i/16)]&1;
    data[floor(i/16)]=data[floor(i/16)]>>1;
  }

}
