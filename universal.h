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

#define MAX_CHAT 3000
#define MESSAGE_SIZE 1024

#define NEW_CLIENT 5
#define REMOVED_CLIENT 6

#define NO_CLIENT -404

#define MAX_NUM_CLIENTS 500

// messaging flags
#define SEND_MESSAGE 1
#define CREATING_CLIENT 2
#define CLOSE_CLIENT -1
#define CLOSE_SERVER -2

// for part 2
#define MAX_NUM_CHANNELS 5000
#define MAX_SIZE_CHANNEL_NAME 1024
#define CREATE_CHANNEL 3
#define CHANGE_CHANNEL 16
#define CLOSE_CHANNEL -3
#define UPDATE_CHANNELS 24
#define KEEP_ALIVE 0

#define ACKNOWLEDGE 4
#define NEW_CLIENT 5
#define REMOVED_CLIENT 6

struct server {
  int num_channels;
  char *channels[];
};

int client_handshake(int *to_server);

// for basic & persistent servers
int server_connect(int from_client);

// for forking server
int server_setup();

// for error handling
int err();

// for generating random numbers
int random_urandom();

#endif
