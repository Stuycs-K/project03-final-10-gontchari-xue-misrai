#include "server.h"

#include <signal.h>

#include "colors.h"
#include "universal.h"
// int to_client, from_client;

int number_of_to_clients = 0, number_of_from_clients = 0,
    new_number_of_from_clients = 0, max_fd = 0;

// active pipes are changed using the select function
fd_set fd_set_of_to_client, fd_set_of_from_client;

int to_client_list[MAX_NUM_CLIENTS], from_client_list[MAX_NUM_CLIENTS];

void handshakes(int *from_client, int *to_client, int *index,
                int *to_client_list, int *from_client_list,
                int *number_of_to_clients, int *number_of_from_clients,
                int *new_number_of_from_clients) {
  // remove the NONBLOCK

  fcntl(*from_client, F_SETFL, fcntl(*from_client, F_GETFL) & ~O_NONBLOCK);

  int random_number = random_random();
  int flag = -1;
  if (read(*from_client, &flag, sizeof(flag)) == -1) err();
  if (flag == CREATING_CLIENT) {
    *to_client = server_connect(*from_client);

    if (write(*to_client, &random_number, sizeof(random_number)) == -1) err();
    printf("[ " HMAG "SERVER" reset
           " ] Created random number %d and sent it to client\n",
           random_number);

    int return_number = -1;
    if (read(*from_client, &return_number, sizeof(return_number)) == -1) err();
    if (return_number - random_number == 1) {
      printf("[ " HMAG "SERVER" reset " ] Got return number %d, " HGRN
             "CORRECTLY" reset " iterated from %d\n",
             return_number, random_number);
    } else {
      printf("THE ITERATED NUMBER: %d is " HRED "NOT CORRECT" reset,
             return_number);
      err();
    }
    //    add the nonblock˝
    fcntl(*from_client, F_SETFL, fcntl(*from_client, F_GETFL) | O_NONBLOCK);
  } else if (flag == CLOSE_CLIENT) {
    printf("[ " HYEL "CHILD SERVER" reset " ]: Client " HRED "DISCONNECT" reset
           "\n");
    close(from_client_list[*index]);
    close(to_client_list[*index]);
    for (int i = *index + 1; i < *number_of_to_clients; i++) {
      to_client_list[i - 1] = to_client_list[i];
    }
    for (int i = *index + 1; i < *number_of_from_clients; i++) {
      from_client_list[i - 1] = from_client_list[i];
    }
    *number_of_from_clients -= 1;
    *number_of_to_clients -= 1;
    *new_number_of_from_clients = *number_of_from_clients;
    (*index)--;
  }
}

int main() {
  signal(SIGPIPE, handle_sigpipe);  // Set up signal handler for SIGPIPE
  signal(SIGINT, handle_sigint);    // Set up signal handler for SIGINT

  if (mkfifo(WKP, 0666) == -1) err();

  FD_ZERO(&fd_set_of_to_client);
  FD_ZERO(&fd_set_of_from_client);

  int initial_from_client = server_setup();
  FD_SET(initial_from_client, &fd_set_of_from_client);
  from_client_list[number_of_from_clients] = initial_from_client;
  max_fd = initial_from_client;
  number_of_from_clients++;

  printf("[ " HYEL "CHILD SERVER" reset " ]: Select server " HGRN "READY" reset
         "\n");
  while (1) {
    reset_fd_sets(&fd_set_of_from_client, &fd_set_of_to_client,
                  number_of_to_clients, number_of_from_clients, to_client_list,
                  from_client_list, &max_fd);
    int num_active_from_clients = select(max_fd + 1, &fd_set_of_from_client,
                                         &fd_set_of_to_client, NULL, NULL);
    if (num_active_from_clients == -1) err();

    for (int i = 0; i < number_of_from_clients; i++) {
      if (FD_ISSET(from_client_list[i], &fd_set_of_from_client)) {
        // remove the NONBLOCK
        int to_client = -1;
        handshakes(&(from_client_list[i]), &to_client, &i, to_client_list,
                   from_client_list, &number_of_to_clients,
                   &number_of_from_clients, &new_number_of_from_clients);

        to_client_list[number_of_to_clients] = to_client;
        if (max_fd < to_client) {
          max_fd = to_client;
        }
        number_of_to_clients++;

        if (unlink(WKP) == -1 || mkfifo(WKP, 0666) == -1) err();

        int new_from_client;
        if ((new_from_client = server_setup()) == -1) err();
        // FD_SET(new_from_client, &fd_set_of_from_client);
        from_client_list[number_of_from_clients] = new_from_client;
        if (max_fd < new_from_client) {
          max_fd = new_from_client;
        }
        new_number_of_from_clients = number_of_from_clients + 1;
      }
    }
    number_of_from_clients = new_number_of_from_clients;

    for (int current_client_index = 0;
         current_client_index < number_of_to_clients; current_client_index++) {
      if (FD_ISSET(to_client_list[current_client_index],
                   &fd_set_of_to_client)) {
        int random_int = abs(random_urandom() % 100);
        if (write(to_client_list[current_client_index], &random_int,
                  sizeof(random_int)) == -1) {
          printf("[ " HYEL "CHILD SERVER" reset " ]: Client " HRED
                 "DISCONNECT" reset " or other error\n");
          close(to_client_list[current_client_index]);
          close(from_client_list[current_client_index]);
          for (int i = current_client_index + 1; i < number_of_to_clients;
               i++) {
            to_client_list[i - 1] = to_client_list[i];
          }
          for (int i = current_client_index + 1; i < number_of_from_clients;
               i++) {
            from_client_list[i - 1] = from_client_list[i];
          }
          number_of_to_clients--;
          number_of_from_clients--;
          new_number_of_from_clients--;
          current_client_index--;
        }
      }
    }
  }
}

void reset_fd_sets(fd_set *fd_set_of_from_client, fd_set *fd_set_of_to_client,
                   int number_of_to_clients, int number_of_from_clients,
                   int to_client_list[MAX_NUM_CLIENTS],
                   int from_client_list[MAX_NUM_CLIENTS], int *max_fd) {
  *max_fd = -1;
  FD_ZERO(fd_set_of_from_client);
  FD_ZERO(fd_set_of_to_client);

  for (int i = 0; i < number_of_to_clients; i++) {
    FD_SET(to_client_list[i], fd_set_of_to_client);
    if (*max_fd < to_client_list[i]) {
      *max_fd = to_client_list[i];
    }
  }

  for (int i = 0; i < number_of_from_clients; i++) {
    FD_SET(from_client_list[i], fd_set_of_from_client);
    if (*max_fd < from_client_list[i]) {
      *max_fd = from_client_list[i];
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
  printf("[ " HRED "SERVER" reset " ]: Caught SIGPIPE, client disconnected\n");
}

/*=========================
  handle_sigint
  args: int sig

  handles and prints a notifying message if it catches a SIGINT signal,
  usually from server disconnect

  returns ABSOLUTELY NOTHING
  =========================*/
void handle_sigint(int sig) {
  printf("[ " HRED "SERVER" reset
         " ]: Caught SIGINT, closing server, number of clients to close: %d\n",
         number_of_to_clients);
  for (int i = 0; i < number_of_to_clients; i++) {
    int close_server_flag = CLOSE_SERVER;
    // add the block here
    printf("[ " HRED "SERVER" reset " ]: Closing client %d\n", i);
    fcntl(to_client_list[i], F_SETFL,
          fcntl(to_client_list[i], F_GETFL) & ~O_NONBLOCK);

    if (write(to_client_list[i], &close_server_flag,
              sizeof(close_server_flag)) == -1) {
      printf("[ " HRED "SERVER" reset " ]: Error closing client %d\n", i);
      err();
    }
  }
  sleep(3);  // give clients a chance to read˝
  for (int i = 0; i < number_of_to_clients; i++) {
    close(to_client_list[i]);
    close(from_client_list[i]);
  }
  close(from_client_list[number_of_from_clients - 1]);
  printf("[ " HRED "SERVER" reset " ]: Caught SIGINT, server disconnected\n");
  exit(0);
}