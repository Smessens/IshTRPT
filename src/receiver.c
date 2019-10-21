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
  uint16_t port;
  char * format;
  char index[argc];  //garde en memoire les arguments non options
  int count=0;  // nombre d'arguments non-options
  int c;
  int i;
  char delim[] = ":";
  char *ptr;
  char hostname;

  for (i=1;i<argc;i++){
    if(strcmp(argv[i],"-m")==0){
      if(i+1>=argc){
        fprintf( stderr,"%s", "Incorrect arguments\n");
        return -1;
      }
      connexionconcurrente=atoi(argv[i+1]);
      i++;
    }
    else if(strcmp(argv[i],"-o")==0){
        if(i+1>=argc){
          fprintf( stderr,"%s", "Incorrect arguments\n");
          return -1;
        }

        format=argv[i+1];
        i++;


    }
    else{
      ptr  = strtok(argv[i], delim);
      if(ptr==NULL){
        port=atoi(argv[i+1]);
        i++;
      }
      else{
        if(strcmp(ptr,argv[1])!=0){
          hostname=*ptr;
          port=atoi(argv[i+1]);
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
  printf("hostname :%c\n",hostname);
  printf("------------------------------------------------------------------------------\n");
  //print test a virer avant la soumission

  struct sockadrr_in6 addresse;
  char error = real_address(hostname,&addresse);

  int sfd = create_socket(&adresse,NULL);

  if (sfd>0&&wait_for_client(sfd)<0){
    close(sfd);
    fprintf(stderr, "Error connecting\n");
    return -1;

  }

  if (sfd==-1){
    fprintf(stderr, "Error connecting\n");
    close(sfd);
    return -1;
  }

  int fd;
  fd =STDOUT_FILENO;
  if(selective(sfd,fd)!=0){
    fprintf(stderr, "Error in selective\n");
  }

  return 0;

}
