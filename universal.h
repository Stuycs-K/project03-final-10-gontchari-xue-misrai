#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define PIPE_SIZING 2048
#ifndef UNIVERSAL_H
#define UNIVERSAL_H
#define WKP "mario"

#define NO_CLIENT -404

#define MAX_NUM_CLIENTS 500

// messaging flags
#define SEND_MESSAGE 1
#define CREATING_CLIENT 2
#define CLOSE_CLIENT -1
#define CLOSE_SERVER -2

// for part 2
#define CREATE_CHANNEL 3
#define CLOSE_CHANNEL -3
#define KEEP_ALIVE 0

struct message {
  int flag;
  char message[10000];
  int num; //optional
};


struct server {
  int num_channels;
  char *channels[];
};

int server_handshake(int *to_client);
int client_handshake(int *to_server);

// for basic & persistent servers
int server_connect(int from_client);

// for forking server
int server_setup();

// for error handling
int err();

// for generating random numbers
int random_random();
int random_urandom();

#endif
