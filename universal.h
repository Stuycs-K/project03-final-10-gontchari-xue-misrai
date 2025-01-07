#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#define PIPE_SIZING 2048
#ifndef UNIVERSAL_H
#define UNIVERSAL_H
#define WKP "mario"

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
