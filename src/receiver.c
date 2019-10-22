#include "selective.h"
#include "packet_implem.h"
#include "read.h"
#include "receiver.h"


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
  char *hostname=NULL;

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
        if(strcmp(ptr,argv[i])!=0){
          hostname=argv[i];
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

  printf("connexion concurrente  :%d\n",connexionconcurrente );   //print test a virer avant la soumission

  printf("port : %d\n",port);
  printf("format :%s\n",format );
  printf("count :%d\n",count );
  printf("hostname :%c\n",hostname);
  printf("------------------------------------------------------------------------------\n");  //print test a virer avant la soumission

    struct sockaddr_in6 *dest_adresse;
    const char * error2 = real_address("::1",dest_adresse); // le 1 c'est nous
    if(error2!=NULL){
       printf("errooooooor 2 %s\n",error2);
    }
    int sfd;
    if (hostname != NULL) {
      
      struct sockaddr_in6 *source_adresse=malloc(sizeof( struct sockaddr_in6));

      const char * error1 = real_address(hostname,source_adresse);
      printf("real adresss post-call\n");
      if (port != 0) {
        sfd = create_socket(source_adresse,port,dest_adresse,0);
      } else {
        sfd = create_socket(source_adresse,0,dest_adresse,0);
      }
      free (source_adresse);
    }
    if (&hostname == NULL) {
      sfd = create_socket(NULL,0,dest_adresse,0);
      if (sfd>0&&wait_for_client(sfd)<0){
        close(sfd);
        fprintf(stderr, "Error connecting\n");
        return -1;
    }
}

    if (sfd==-1){
      fprintf(stderr, "Error connecting\n");
      close(sfd);
      return -1;
    }

    int fd;    //fd =STDOUT_FILENO;

    fd = 0; //test

    printf(" préselective\n");

    if(selective(sfd,fd)!=0){
      fprintf(stderr, "Error in selective\n");
    }
    return 0;

  }

