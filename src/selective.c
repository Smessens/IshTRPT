#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h> /* * sockaddr_in6 */
#include <sys/types.h> /* sockaddr_in6 */
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <errno.h>

void read_write_loop(const int sfd) {
  printf("welcome in read_write_loop\n");
  int retval = sfd+1;
  fd_set fd_read;
  int error;
  char buffIn[1024];
  char buffOut[1024];
  int finished = 0;

  struct timeval tv;
  tv.tv_sec = 5;
  tv.tv_usec = 0;

  while (finished==0) {
    printf("coucou\n");
    memset((void *)buffIn, 0, 1024);
    memset((void *)buffOut, 0 , 1024);

    FD_ZERO(&fd_read);
    FD_SET(0,&fd_read); //0 is stdin
    FD_SET(sfd,&fd_read);
    retval = select(retval, &fd_read, NULL, NULL, &tv);
    if (retval == -1) {
      fprintf(stderr, "issue with select\n");
      break;
    }
    // from STDIN to sfd
    if (FD_ISSET(0,&fd_read)) { //0 is stdin
      error = read(0, buffIn, 1024); //0 is stdin
      if (error > 0) {
        error = write(sfd,buffIn,error);
        if (error < 0) {
          fprintf(stderr, "issue in writing in the socket from STDIN\n");
        }
      }
    }

    // from sfd to STDOUT
    if (FD_ISSET(sfd,&fd_read)) {
      error = read(sfd, buffOut, 1024);
      if (error > 0) {
        error = write(1,buffOut,error); //1 is stdout
        if (error < 0) {
          fprintf(stderr, "issue in writing in STDOUT from the socket\n");
        }
      }
    }
    finished = feof(stdin);
  }
  printf("GB from read_write_loop\n");
}
