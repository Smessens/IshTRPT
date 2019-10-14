#include "packet_interface.h"

/* Extra #includes */
/* Your code will be inserted here */

struct __attribute__((__packed__)) pkt {
  uint8_t type : 2;
  uint8_t tr : 1;
  uint8_t window : 5;
  uint8_t l : 1;
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
  new->l = 0;
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
    char buffer = data[0]; // recup 2 first bytes
    uint8_t l = buffer & 1; //recup l
    if (l == 1) { // => length sur 15 bits
      uint32_t crc_header = (uint16_t) data[5] << 8;
      crc_header += (uint16_t) data[6];
    }
}

pkt_status_code pkt_encode(const pkt_t* pkt, char *buf, size_t *len)
{
    /* Your code will be inserted here */
}

ptypes_t pkt_get_type  (const pkt_t* pkt)
{
    /* Your code will be inserted here */
}

uint8_t  pkt_get_tr(const pkt_t* pkt)
{
    /* Your code will be inserted here */
}

uint8_t  pkt_get_window(const pkt_t* pkt)
{
    /* Your code will be inserted here */
}

uint8_t  pkt_get_seqnum(const pkt_t* pkt)
{
    /* Your code will be inserted here */
}

uint16_t pkt_get_length(const pkt_t* pkt)
{
    /* Your code will be inserted here */
}

uint32_t pkt_get_timestamp   (const pkt_t* pkt)
{
    /* Your code will be inserted here */
}

uint32_t pkt_get_crc1   (const pkt_t* pkt)
{
    /* Your code will be inserted here */
}

uint32_t pkt_get_crc2   (const pkt_t* pkt)
{
    /* Your code will be inserted here */
}

const char* pkt_get_payload(const pkt_t* pkt)
{
    /* Your code will be inserted here */
}


pkt_status_code pkt_set_type(pkt_t *pkt, const ptypes_t type)
{
    /* Your code will be inserted here */
}

pkt_status_code pkt_set_tr(pkt_t *pkt, const uint8_t tr)
{
    /* Your code will be inserted here */
}

pkt_status_code pkt_set_window(pkt_t *pkt, const uint8_t window)
{
    /* Your code will be inserted here */
}

pkt_status_code pkt_set_seqnum(pkt_t *pkt, const uint8_t seqnum)
{
    /* Your code will be inserted here */
}

pkt_status_code pkt_set_length(pkt_t *pkt, const uint16_t length)
{
    /* Your code will be inserted here */
}

pkt_status_code pkt_set_timestamp(pkt_t *pkt, const uint32_t timestamp)
{
    /* Your code will be inserted here */
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
                                const uint16_t length)
{
    /* Your code will be inserted here */
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
