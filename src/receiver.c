#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <string.h>

int main (int argc, char **argv)
{
  int connexionconcurrente=-1;
  int port;
  char * format;
  char index[argc];  //garde en memoire les arguments non options
  int count=0;  // nombre d'arguments non-options
  int c;
  int i;
  for (i=1;i<argc;i++){
    if(strcmp(argv[i],"-m")==0){
      if(i+1>=argc){
        fprintf( stderr,"%s", "Incorrect arguments\n");
        return -1;
      }
      connexionconcurrente=atoi(argv[i+1]);
      i++;
    }
    else{
      if(strcmp(argv[i],"::")==0){
        if(i+1>=argc){
          fprintf( stderr,"%s", "Incorrect arguments\n");
          return -1;
        }
        port=atoi(argv[i+1]);
        i++;
      }
      else{
        if(strcmp(argv[i],"-o")==0){
          if(i+1>=argc){
            fprintf( stderr,"%s", "Incorrect arguments\n");
            return -1;
          }

          format=argv[i+1];
          i++;
        }
        else{
          index[count]=i;
          count++;
        }
      }
    }
  }

  for (i=0; i<count; i++){
    fprintf (stderr,"Non-option argument %s\n", argv[index[i]]);
  }

  //print test a virer avant la soumission
  printf("connexion concurrente  :%d\n",connexionconcurrente );
  printf("port : %d\n",port);
  printf("format :%s\n",format );
  printf("count :%d\n",count );
  printf("argc :%d\n",argc );
  printf("------------------------------------------------------------------------------\n");
  //print test a virer avant la soumission


  //open file


}
