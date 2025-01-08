#include <signal.h>

#include "colors.h"
#include "universal.h"
int to_client, from_client;

pid_t a = -1;

/*=========================
  handle_sigpipe
  args: int sig

  handles and prints a notifying message if it catches a SIGPIPE signal, usually from a child disconnect

  returns ABSOLUTELY NOTHING
  =========================*/
void handle_sigpipe(int sig) {
  printf("(" HRED "SERVER" reset "): Caught SIGPIPE, client disconnected\n");
}

/*=========================
  handle_sigint
  args: int sig

  handles and prints a notifying message if it catches a SIGINT signal, usually from server disconnect

  returns ABSOLUTELY NOTHING
  =========================*/
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
  signal(SIGINT, handle_sigint);  // Set up signal handler for SIGINT
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
