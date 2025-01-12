#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "colors.h"
#include "universal.h"

// UPSTREAM = to the server / from the client
// DOWNSTREAM = to the client / from the server
/*=========================
  server_setup

  creates the WKP and opens it, waiting for a  connection.
  removes the WKP once a connection has been made

  returns the file descriptor for the upstream pipe.
  =========================*/
int server_setup() {
  printf("(" HMAG "SERVER SETUP" reset
         ") Created the client pipe in the server setup function\n");

  int from_client = open(WKP, O_RDONLY | O_NONBLOCK, 0666);
  if (from_client == -1) err();
  return from_client;
}

/*=========================
  server_handshake
  args: int * to_client

  Performs the server side pipe 3 way handshake.
  Sets *to_client to the file descriptor to the downstream pipe (Client's
  private pipe).

  returns the file descriptor for the upstream pipe (see server setup).
  =========================*/
int server_handshake(int *to_client) {
  //   set up the private pipe here
  int from_client = server_setup();
  *to_client = server_connect(from_client);
  if (*to_client == -1) err();

  int random_number = random_random();
  if (write(*to_client, &random_number, sizeof(random_number)) == -1) err();
  printf("(" HMAG "SERVER" reset
         ") Created random number %d and sent it to client\n",
         random_number);

  int return_number = -1;
  if (read(from_client, &return_number, sizeof(return_number)) == -1) err();
  printf("(" HMAG "SERVER" reset
         ") Got return number %d, which is hopefully iterated from %d\n",
         return_number, random_number);

  return from_client;
}

/*=========================
  client_handshake
  args: int * to_server

  Performs the client side pipe 3 way handshake.
  Sets *to_server to the file descriptor for the upstream pipe.

  returns the file descriptor for the downstream pipe.
  =========================*/
int client_handshake(int *to_server) {
  // gets the file descriptor for the pipe of the server
  //   also write the number the server
  // but then read back from the pipe...
  // gets the and then returns file descriptor of the upstream pipe? creates it,
  // or does it get from another function

  printf("(" HCYN "CLIENT" reset "): Creating fifo\n");
  char fifo_name[PIPE_SIZING] = {"\0"};
  sprintf(fifo_name, "%d", getpid());
  char *fifo_ending = ".fifo";
  strcat(fifo_name, fifo_ending);
  if (mkfifo(fifo_name, 0666) == -1) err();

  printf("(" HCYN "CLIENT" reset
         "): Sending the number %d.fifo (the pipe_name) to the parent "
         "pipe\n",
         getpid());

  *to_server = -1;
  while (*to_server == -1) {
    *to_server = open(WKP, O_WRONLY, 0666);
  }

  printf("Got here!\n");
  if (write(*to_server, fifo_name, strlen(fifo_name)) == -1) err();
  printf("Got there!\n");

  printf("(" HCYN "CLIENT" reset
         "): Reading from private pipe to get the int\n");
  int pipe_buff;
  int from_server = open(fifo_name, O_RDONLY, 0666);
  if (from_server == -1) err();

  // attempt to read random number from parent pipe
  if (read(from_server, &pipe_buff, sizeof(pipe_buff)) == -1) err();
  printf("(" HCYN "CLIENT" reset "): Read the random int, got %d\n", pipe_buff);

  //   removing the client pipe
  if (remove(fifo_name) == -1) err();

  pipe_buff++;

  printf("(" HCYN "CLIENT" reset
         "): Iterated the number, now sending it to the public pipe\n");
  if (write(*to_server, &pipe_buff, sizeof(pipe_buff)) == -1) err();

  printf("(" HCYN "CLIENT" reset "): Sent the number\n");

  return from_server;
}

/*=========================
  server_connect
  args: int from_client

  handles the subserver portion of the 3 way handshake

  returns the file descriptor for the downstream pipe.
  =========================*/
int server_connect(int from_client) {
  char fifo_name_buff[PIPE_SIZING] = {"\0"};
  int ret = read(from_client, fifo_name_buff, sizeof(fifo_name_buff));
  if (ret == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return NO_CLIENT;
    }
    err();
  }
  if (ret == 0) {
    return NO_CLIENT;
  }
  printf("(" HMAG "SERVER" reset ") Read from the WKP, got fifo name %s \n",
         fifo_name_buff);
  int to_client = open(fifo_name_buff, O_WRONLY, 0666);
  return to_client;
}

/*=========================
  err()
  args: none

  prints error messages if we ever encounter something

  returns ABSOLUTELY NOTHING
  =========================*/
int err() {
  printf("\x1b[31m errno %d\x1b[0m\n", errno);
  printf("%s\n", strerror(errno));
  exit(1);
}

/*=========================
  random_random()
  args: none

  gets a random integer from dev/random to be used in code

  returns random int
  =========================*/
int random_random() {
  int r_file = open("/dev/random", O_RDONLY, 0);
  if (r_file == -1) err();
  int bytes;
  int read_result = read(r_file, &bytes, sizeof(bytes));
  if (read_result == -1) err();
  close(r_file);
  return bytes;
}

/*=========================
  random_urandom()
  args: none

  gets a random integer from dev/urandom to be used in code

  returns random int
  =========================*/
int random_urandom() {
  int r_file = open("/dev/urandom", O_RDONLY, 0);
  if (r_file == -1) err();
  int bytes;
  int read_result = read(r_file, &bytes, sizeof(bytes));
  if (read_result == -1) err();
  close(r_file);
  return bytes;
}
