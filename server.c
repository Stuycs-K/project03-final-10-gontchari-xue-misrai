#include <signal.h>

#include "colors.h"
#include "universal.h"
int to_client, from_client;

pid_t a = -1;

void handle_sigpipe(int sig) {
  printf("(" HRED "SERVER" reset "): Caught SIGPIPE, client disconnected\n");
}

void handle_sigint(int sig) {
  if (a != 0) {
    if (unlink(WKP) != 0) err();
    printf("(" HRED "SERVER" reset "): Closing down server due to " HRED
           "SIGINT" reset "\n");
    close(from_client);
  } else {
    printf("(" HRED "SERVER" reset "): Closing child processes\n");
    int close_num = -1;
    if (write(to_client, &close_num, sizeof(close_num)) == -1) err();
    close(to_client);
  }
  exit(0);
}

int main() {
  signal(SIGPIPE, handle_sigpipe);  // Set up signal handler for SIGPIPE
  signal(SIGINT, handle_sigint);
  while (1) {
    if (a != 0) {
      from_client = server_handshake(&to_client);  // initial handshake
      printf("(" HMAG "SERVER" reset
             "): Connection established on the server side\n");
      a = fork();
    }

    if (a == 0) {
      printf("(" HYEL "CHILD SERVER" reset "): Handed off to child\n");
      while (1) {
        int random_int = abs(random_urandom() % 100);
        if (write(to_client, &random_int, sizeof(random_int)) == -1) {
          printf("(" HYEL "CHILD SERVER" reset "): Client " HRED
                 "DISCONNECT" reset " or other error\n");
          close(to_client);
          exit(0);
        }
        sleep(1);
      }
    }
  }
}


int err() {
  printf("\x1b[31m errno %d\x1b[0m\n", errno);
  printf("%s\n", strerror(errno));
  exit(1);
}

int random_random() {
  int r_file = open("/dev/random", O_RDONLY, 0);
  if (r_file == -1) err();
  int bytes;
  int read_result = read(r_file, &bytes, sizeof(bytes));
  if (read_result == -1) err();
  return bytes;
}

int random_urandom() {
  int r_file = open("/dev/urandom", O_RDONLY, 0);
  if (r_file == -1) err();
  int bytes;
  int read_result = read(r_file, &bytes, sizeof(bytes));
  if (read_result == -1) err();
  return bytes;
}
