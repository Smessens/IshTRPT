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

  FILE * log;
  log = fopen ("log.txt", "w");

  for (i=1;i<argc;i++){
    if(strcmp(argv[i],"-m")==0){ // test connexion concurrente
      if(i+1>=argc){
        fprintf(log,"%s", "Incorrect arguments with -m (receiver)\n");
        return -1;
      }
      connexionconcurrente=atoi(argv[i+1]);
      i++;
    }
    else if(strcmp(argv[i],"-o")==0){ // test format
      if(i+1>=argc){
        fprintf(log,"%s", "Incorrect arguments with -o (receiver)\n");
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
    fprintf (log,"Non-option argument %s (receiver)\n", argv[index[i]]);
  }

  printf("connexion concurrente  :%d\n",connexionconcurrente );   //print test a virer avant la soumission

  printf("port : %d\n",port);
  printf("format :%s\n",format );
  printf("count :%d\n",count );
  printf("hostname :%s\n",hostname);
  printf("------------------------------------------------------------------------------\n");  //print test a virer avant la soumission

  //get true format
  delim="%";
  ptr=strtok(format,delim);
  if (0!=strcmp(format,ptr){
  }

  struct sockaddr_in6 *dest_adresse;
  const char * error1 = real_address("localhost",&dest_adresse); // le 1 c'est nous
  if(error1!=NULL){
    fprintf(log,"first real_address issue : %s (receiver)\n",error1);
  }
  int sfd;
  if (hostname != NULL) {

    struct sockaddr_in6 *source_adresse=malloc(sizeof( struct sockaddr_in6));

    const char * error2 = real_address(hostname,&source_adresse);
    if(error2!=NULL){
      fprintf(log,"second real_address issue : %s (receiver)\n",error2);
    }
    if (port != 0) {
      sfd = create_socket(source_adresse,port,&dest_adresse,port);
    } else {
      sfd = create_socket(source_adresse,port,&dest_adresse,port);
    }
    free (source_adresse);
  }
  if (hostname ==NULL) {
    printf("adresse = ::\n");
    sfd = create_socket(&dest_adresse,port,NULL,port);
    //   char hostnamebis[50];
    //   struct sockaddr * addr;
    //    int *adlen;
    //     getpeername(sfd,addr,adlen);
    //      printf("hostnamebis%d\n ", addr->sin_addr);
    if (sfd>0&&wait_for_client(sfd)<0){
      fprintf(log,"%s","Error connecting (receiver)\n");
      fclose(log);
      close(sfd);
      return -1;
    }
  }

  if (sfd==-1){
    fprintf(log,"%s","Error connecting (receiver)\n");
    fclose(log);
    close(sfd);
    return -1;
  }

  int fd;    //fd =STDOUT_FILENO;
  // fileno(FILE *) -> give us de file descriptor of the FILE* got from fopen

  fd = 0; //test

  if(selective(sfd,fd,log)!=0){
    fprintf(log,"Error in selective (receiver)\n");
  }
  fclose(log);
  return 0;
}
