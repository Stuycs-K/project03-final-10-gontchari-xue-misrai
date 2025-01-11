#include "server.h"

#include <signal.h>

#include "colors.h"
#include "universal.h"
// int to_client, from_client;

int number_of_to_clients = 0, number_of_from_clients = 0, max_fd_to_client = -1,
    max_fd_from_server = -1, new_number_of_from_clients = 0;

// active pipes are changed using the select function
fd_set fd_set_of_to_client, active_fd_set_of_to_client, fd_set_of_from_client,
    active_fd_set_of_from_client;

int to_client_list[MAX_NUM_CLIENTS], from_client_list[MAX_NUM_CLIENTS];

int main() {
  signal(SIGPIPE, handle_sigpipe);  // Set up signal handler for SIGPIPE
  signal(SIGINT, handle_sigint);    // Set up signal handler for SIGINT

  if (mkfifo(WKP, 0666) == -1) err();

  FD_ZERO(&fd_set_of_to_client);
  FD_ZERO(&active_fd_set_of_to_client);
  FD_ZERO(&fd_set_of_from_client);
  FD_ZERO(&active_fd_set_of_from_client);

  int initial_from_client = server_setup();
  FD_SET(initial_from_client, &fd_set_of_from_client);
  from_client_list[number_of_from_clients] = initial_from_client;
  max_fd_from_server = initial_from_client;
  number_of_from_clients++;

  printf("(" HYEL "CHILD SERVER" reset "): Select server " HGRN "READY" reset
         "\n");
  while (1) {
    // for replacements since select *does stuff*
    // FD_ZERO(&active_fd_set_of_from_client);
    // FD_ZERO(&active_fd_set_of_to_client);
    memcpy(&active_fd_set_of_from_client, &fd_set_of_from_client,
           sizeof(fd_set));
    memcpy(&active_fd_set_of_to_client, &fd_set_of_to_client, sizeof(fd_set));

    int max_fd = (max_fd_from_server > max_fd_to_client) ? max_fd_from_server
                                                         : max_fd_to_client;

    int num_active_from_clients =
        select(max_fd + 1, &active_fd_set_of_from_client,
               &active_fd_set_of_to_client, NULL, NULL);
    if (num_active_from_clients == -1) err();
    for (int current_from_client_index = 0;
         current_from_client_index < number_of_from_clients;
         current_from_client_index++) {
      if (FD_ISSET(from_client_list[current_from_client_index],
                   &active_fd_set_of_from_client)) {
        fcntl(from_client_list[current_from_client_index], F_SETFL,
              fcntl(from_client_list[current_from_client_index], F_GETFL) &
                  ~O_NONBLOCK);

        int to_client =
            server_connect(from_client_list[current_from_client_index]);
        int random_number = random_random();
        if (write(to_client, &random_number, sizeof(random_number)) == -1)
          err();
        printf("(" HMAG "SERVER" reset
               ") Created random number %d and sent it to client\n",
               random_number);

        int return_number = -1;
        if (read(from_client_list[current_from_client_index], &return_number,
                 sizeof(return_number)) == -1)
          err();
        printf("(" HMAG "SERVER" reset
               ") Got return number %d, which is hopefully iterated from %d\n",
               return_number, random_number);
        fcntl(from_client_list[current_from_client_index], F_SETFL,
              fcntl(from_client_list[current_from_client_index], F_GETFL) |
                  O_NONBLOCK);

        FD_SET(to_client, &fd_set_of_to_client);
        to_client_list[number_of_to_clients] = to_client;
        if (max_fd_to_client < to_client) {
          max_fd_to_client = to_client;
        }
        number_of_to_clients++;

        if (remove(WKP) == -1) err();
        if (mkfifo(WKP, 0666) == -1) err();
        int new_from_client = server_setup();
        if (new_from_client == -1) err();
        FD_SET(new_from_client, &fd_set_of_from_client);
        from_client_list[number_of_from_clients] = new_from_client;
        if (max_fd_from_server < new_from_client) {
          max_fd_from_server = new_from_client;
        }
        new_number_of_from_clients = number_of_from_clients + 1;
      }
    }
    number_of_from_clients = new_number_of_from_clients;
    // printf("Number of to_clients: %d \n", number_of_to_clients);
    // printf("Number of from_clients: %d\n", number_of_from_clients);
    // FD_ZERO(&active_fd_set_of_from_client);
    // FD_ZERO(&active_fd_set_of_to_client);
    // active_fd_set_of_from_client = fd_set_of_from_client;
    // active_fd_set_of_to_client = fd_set_of_to_client;

    // max_fd = (max_fd_from_server > max_fd_to_client) ? max_fd_from_server
    //                                                  : max_fd_to_client;

    // num_active_from_clients = select(max_fd + 1,
    // &active_fd_set_of_from_client,
    //                                  &active_fd_set_of_to_client, NULL,
    //                                  NULL);

    for (int current_client_index = 0;
         current_client_index < number_of_to_clients; current_client_index++) {
      printf("current client index: %d\n",
             current_client_index);
      if (FD_ISSET(to_client_list[current_client_index],
                   &active_fd_set_of_to_client)) {
        int random_int = abs(random_urandom() % 100);
        if (write(to_client_list[current_client_index], &random_int,
                  sizeof(random_int)) == -1) {
          printf("(" HYEL "CHILD SERVER" reset "): Client " HRED
                 "DISCONNECT" reset " or other error\n");
        }
      }
    }
  }
}

/*=========================
  handle_sigpipe
  args: int sig

  handles and prints a notifying message if it catches a SIGPIPE signal,
  usually from a child disconnect

  returns ABSOLUTELY NOTHING
  =========================*/
void handle_sigpipe(int sig) {
  printf("(" HRED "SERVER" reset "): Caught SIGPIPE, client disconnected\n");
  exit(0);
}

/*=========================
  handle_sigint
  args: int sig

  handles and prints a notifying message if it catches a SIGINT signal,
  usually from server disconnect

  returns ABSOLUTELY NOTHING
  =========================*/
void handle_sigint(int sig) {
  //   if (a != 0) {
  //     if (unlink(WKP) != 0) err();
  //     printf("(" HRED "SERVER" reset "): Closing down server due to "
  //     HRED
  //            "SIGINT" reset "\n");
  //     close(from_client);
  //   } else {
  //     printf("(" HRED "SERVER" reset "): Closing child processes\n");
  //     int close_num = CLOSE_SERVER;
  //     if (write(to_client, &close_num, sizeof(close_num)) == -1) err();
  //     close(to_client);
  //   }
  exit(0);
}
