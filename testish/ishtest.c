
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>



int main(int argc,char *argv[]){

  char a='4';
  char b= '1';
  uint8_t c= a;

  printf("%d\n",c);

  uint32_t crc= (uint16_t) a<<7;

  printf("%d\n",crc);

  crc += (uint16_t)b;

/*
  uint8_t t = a &1 ;
  t=t<<7;
  t+=a &1 ;
*/

  printf("%d",crc);

  printf("\n hello world \n");
  return 0;
}
