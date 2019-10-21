#ifndef __SELECTIVE_H_
#define __SELECTIVE_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h> /* * sockaddr_in6 */
#include <sys/types.h> /* sockaddr_in6 */
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <errno.h>

#include "packet_implem.h"
#include "selective.h"
#include "read.h"

int main (int argc, char **argv);

#endif
