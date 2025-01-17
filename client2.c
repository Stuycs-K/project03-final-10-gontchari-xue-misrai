#include <pwd.h>
#include <signal.h>
#include <unistd.h>

#include "client.h"
#include "colors.h"
#include "universal.h"

int to_server, from_server;

char signature[256];

/*=========================
  main

  handles client side pipe processing

  returns ABSOLUTELY NOTHING
  =========================*/
int main() {
  char *usrnme;
  // usrnme=(char *)calloc(50,sizeof(char));
  uid_t x = getuid();
  struct passwd *y = getpwuid(x);
  usrnme = y->pw_name;
  // cuserid(signature);
  // printf("USERID: %s\n", usrnme);
  char pid[256];
  sprintf(pid, "@%d: ", getpid());
  strcat(signature, usrnme);
  strcat(signature, pid);
  // printf("TESTING SIGNATURE HERE: %s\n", signature);

  signal(SIGINT, handle_sigint);
  from_server = client_handshake(&to_server);
  printf("[ " HCYN "CLIENT" reset " ]: Client " HGRN "ESTABLISHED" reset
         " by the server\n");

  char chat_history[MAX_CHAT];
  read(from_server, &chat_history, MAX_CHAT);
  printf("%s\n", chat_history);

  while (1) {
    int randomized_number = 0;
    int ret;

    // should be waiting for user input here instead
    /*
    char *user_input = "user input";

    int flag = SEND_MESSAGE;
    write(to_server, &flag, sizeof(int));
    write(to_server, &user_input, strlen(user_input));
    */

    // if (ret == 0) err();
    if (randomized_number ==
        CLOSE_SERVER) {  // the same descrbed in Universal.h
      printf("[ " HCYN "CLIENT" reset " ]: Detected pipe " HRED "CLOSURE" reset
             " by server; closing down\n");
      close(to_server);
      close(from_server);
      exit(0);
    }
  }
}

/*=========================
  handle_sigint
  args: int sig

  handles and prints a notifying message if it catches a SIGINT signal, and
  closes client

  returns ABSOLUTELY NOTHING
  =========================*/
void handle_sigint(int sig) {
  int flag = CLOSE_CLIENT;
  if (write(to_server, &flag, sizeof(flag)) == -1) err();
  
  close(to_server);
  close(from_server);
  char fifo_name[PIPE_SIZING] = {"\0"};
  sprintf(fifo_name, "%d", getpid());
  char *fifo_ending = ".fifo";
  strcat(fifo_name, fifo_ending);
  unlink(fifo_name);

  exit(0);
}
