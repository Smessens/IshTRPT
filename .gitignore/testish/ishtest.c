
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include <zlib.h>

int btoi(int* buffer, int begin, int end) {
	int r = buffer[end];
	for(int i = 1; i <= end-begin; i++) {
		r += pow(buffer[end-i]*2,i);
	}
  return r;
}

int main(int argc,char *argv[]){
  char * data = (char *) malloc(sizeof(char)*10);
  for(int i = 0; i<10;i++) {
    data[i] = i;
  }
  //uLong crc = crc32(0L,Z_NULL,0);
  uLong crc = crc32(0L,Z_NULL,0);
	memcpy((void *)&crc,(const void *)&data[0],4);


  printf("\n hello world \n");
  return 0;
}
