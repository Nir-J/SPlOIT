#ifndef SPLOIT_H
#define SPLOIT_H

#define DEBUG true

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/stat.h>

#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

/* Connects to the specified IP and port */
int connect_to_socket(int port, char * ip);


/* Function to create a listening connection and return socket descriptor */
int listening_socket(int port, int reuse);


#endif
