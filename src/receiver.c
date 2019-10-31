#include "selective.h"
#include "packet_implem.h"
#include "read.h"
#include "receiver.h"



int main (int argc, char **argv)
{
  int connexionconcurrente=-1;
  uint16_t port;
  char * format=NULL;
  char index[argc];  //garde en memoire les arguments non options
  int count=0;  // nombre d'arguments non-options
  int c;
  int i;
  char delim[] = ":";
  char *ptr;
  char *hostname=NULL;
  char *log_name=NULL;

  for (i=1;i<=argc;i++){
    printf("%s ", argv[i]);
  }
  printf("\n argc : %d\n", argc);
  for (i=1;i<argc;i++){
    if(strcmp(argv[i],"-m")==0){
      if(i+1>=argc){
        fprintf(stderr,"%s", "Incorrect arguments with -m (receiver)\n");
        return -1;
      }
      connexionconcurrente=atoi(argv[i+1]);
      i++;
    }
    else if(strcmp(argv[i],"-o")==0){
      if(i+1>=argc){
        fprintf(stderr,"%s", "Incorrect arguments with -o (receiver)\n");
        return -1;
      }
      format=argv[i+1];
      i++;
    }
    else if(strcmp(argv[i],"2>")==0) {
      printf("found 2> \n");
      if(i+1>=argc) {
        fprintf(stderr,"%s", "Incorrect arguments with 2> (receiver)\n");
      }
      log_name=argv[i+1];
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
    fprintf (stderr,"Non-option argument (receiver) : %s\n", argv[index[i]]);
  }

  printf("connexion concurrente  :%d\n",connexionconcurrente );   //print test a virer avant la soumission

  printf("port : %d\n",port);
  printf("format :%s\n",format );
  printf("count :%d\n",count );
  printf("hostname :%s\n",hostname);
  printf("log_name :%s\n",log_name);
  printf("------------------------------------------------------------------------------\n");  //print test a virer avant la soumission


  // log
  FILE * log = stderr;

  if(log_name != NULL) {
    log = fopen(log_name, "w");
  }

  //get real format
  if(format!=NULL){
    int formatsize =strlen(format);
    char formatbuffer[formatsize];
    char formatbuffer2[formatsize];
    char *token;
    uint16_t formatnumber=0;

    token = strtok(format, "%");

    strcpy(formatbuffer,token); //premiere partie

    token=strtok(NULL, "%");

    token = strtok(token,"d");
    token = strtok(NULL," ");

    strcat(formatbuffer,formatnumber);
    strcpy(formatbuffer2,token);
    strcat(formatbuffer,formatbuffer2);
    strcpy(format,formatbuffer);
  }
  printf("realformat :%s\n",format);

  int fdo = 1;
  if(format != NULL) {
    FILE * fileout = fopen(format,"w");
    fdo = fileno(fileout);
  }
  else {
    FILE * fileout = fopen("fileout.dat","w");
    fdo = fileno(fileout);
  }

  struct sockaddr_in6 dest_adresse;
  const char * error2 = real_address("localhost",&dest_adresse); // le 1 c'est nous
  if(error2!=NULL){
    fprintf(log,"first real_address issue (receiver): %s\n",error2);
  }
  int sfd;
  if (hostname != NULL) {

    struct sockaddr_in6 *source_adresse=malloc(sizeof( struct sockaddr_in6));

    const char * error1 = real_address(hostname,source_adresse);
    if(error1!=NULL){
      fprintf(log,"second real_address issue (receiver): %s\n",error1);
    }
    if (port != 0) {
      sfd = create_socket(source_adresse,port,&dest_adresse,port);
    } else {
      sfd = create_socket(source_adresse,port,&dest_adresse,port);
    }
    free (source_adresse);
  }
  if (hostname ==NULL) {
    sfd = create_socket(&dest_adresse,port,NULL,port);
    if (sfd>0&&wait_for_client(sfd)<0){
      close(sfd);
      fprintf(log, "Error connecting (receiver)\n");
      fclose(log);
      return -1;
    }
  }

  if (sfd==-1){
    fprintf(log, "Error connecting (receiver)\n");
    fclose(log);
    close(sfd);
    return -1;
  }

  if(selective(sfd,fdo,log)!=0){
    fprintf(log, "Error in selective (receiver)\n");
  }
  return 0;

}
