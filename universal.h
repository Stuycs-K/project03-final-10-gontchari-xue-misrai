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

#define KEY 39682

union semun { 
    int  val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO */
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

// for closing server
void server_close();

// for error handling
int err();

// for generating random numbers
int random_random();
int random_urandom();

#endif
