#include "server.h"

#include <signal.h>

#include <sys/select.h>
#include "colors.h"
#include "universal.h"

// max_fd keeps track of the max_fd of both to_client and from_client, for the
// select function

int number_of_to_clients = 0, number_of_from_clients = 0,
    new_number_of_from_clients = 0, max_fd = 0;

// active pipes are changed using the select function
fd_set fd_set_of_to_client, fd_set_of_from_client;

// the list of file descrptors
int to_client_list[MAX_NUM_CLIENTS], from_client_list[MAX_NUM_CLIENTS];

char chatHistory[MAX_CHAT] = {0};

int number_of_channels = 1;
char * chatHistories[MAX_NUM_CHANNELS];
int currChannels[MAX_NUM_CLIENTS];
char * channelNames[MAX_NUM_CHANNELS];

int main() {
  // handle the sigpipe and signit signals
  signal(SIGPIPE, handle_sigpipe);
  signal(SIGINT, handle_sigint);

  for(int i = 0; i < MAX_NUM_CHANNELS; i++){
    chatHistories[i] = (char *)calloc(MAX_CHAT, sizeof(char));
    channelNames[i] = NULL;
  }
  char first[10] = "general";
  chatHistories[0] = (char *)calloc(MAX_SIZE_CHANNEL_NAME, sizeof(char));
  strcpy(channelNames[0], first);

  for (int i = 0; i < MAX_NUM_CLIENTS; i++) {
    currChannels[i] = 0;
  }

  if (mkfifo(WKP, 0666) == -1) err();

  //   initalize the fd_sets through fd_zero
  FD_ZERO(&fd_set_of_to_client);
  FD_ZERO(&fd_set_of_from_client);

  //   create the initial pipe and iterate all the variables
  int initial_from_client = server_setup();
  FD_SET(initial_from_client, &fd_set_of_from_client);
  from_client_list[number_of_from_clients] = initial_from_client;
  max_fd = initial_from_client;
  number_of_from_clients++;

  printf("[ " HYEL "SERVER" reset " ]: Select server " HGRN "READY" reset "\n");
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
        handle_from_client(&(from_client_list[i]), &to_client, &i,
                           to_client_list, from_client_list,
                           &number_of_to_clients, &number_of_from_clients,
                           &new_number_of_from_clients, &max_fd);
      }
    }
    // this links back so that we don't have infinite recoursion with the number
    // of clients increasing ad infinitum
    number_of_from_clients = new_number_of_from_clients;
  }
}

/*=========================
  reset_fd_sets
  args:
    fd_set *fd_set_of_from_client
    fd_set *fd_set_of_to_client
    int number_of_to_clients
    int number_of_from_clients
    int to_client_list[MAX_NUM_CLIENTS]
    int from_client_list[MAX_NUM_CLIENTS]
    int *max_fd

resets the fd_sets and does some error checking, such as making sure that the
file descriptors don't have any -1s

  returns ABSOLUTELY NOTHING
  =========================*/

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
  handle_from_client
  args:
    int *from_client
    int *to_client
    int *index,
    int *to_client_list
    int *from_client_list
    int *number_of_to_clients
    int *number_of_from_clients
    int *new_number_of_from_clients
    int *max_fd

    handle input from client (e.g. the handshakes, or the messaging), will also
  update global variables

  returns ABSOLUTELY NOTHING
  =========================*/

void handle_from_client(int *from_client, int *to_client, int *index,
                        int *to_client_list, int *from_client_list,
                        int *number_of_to_clients, int *number_of_from_clients,
                        int *new_number_of_from_clients, int *max_fd) {
  // remove the NONBLOCK
  fcntl(*from_client, F_SETFL, fcntl(*from_client, F_GETFL) & ~O_NONBLOCK);

  //   flags are sent by the client to the server and specified in universal.h
  int flag = -1;
  if (read(*from_client, &flag, sizeof(flag)) == -1) err();
  if (flag == CREATING_CLIENT) {
    // printf("NUM CLIENTS: %d\n", *number_of_to_clients);
    // adds the clients and creates the server
    // three way handshake
    int random_number = random_random();
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
    //    add the nonblock
    printf("[ " HYEL "SERVER" reset " ]: Client " HGRN "CONNECTED" reset "\n");
    if (write(*to_client, chatHistories[0], MAX_CHAT) == -1) err();
    printf("Sent chat history to client\n");

    // end the three way handshake
    // add the new file descriptors to the list (the fd_sets will be
    // reinitialized at the beginning of the next loop)
    to_client_list[*number_of_to_clients] = *to_client;
    if (*max_fd < *to_client) {
      *max_fd = *to_client;
    }
    (*number_of_to_clients)++;

    if (unlink(WKP) == -1 || mkfifo(WKP, 0666) == -1) err();

    int new_from_client;
    if ((new_from_client = server_setup()) == -1) err();
    from_client_list[*number_of_from_clients] = new_from_client;
    if (*max_fd < new_from_client) {
      *max_fd = new_from_client;
    }
    *new_number_of_from_clients = *number_of_from_clients + 1;
    printf("NUM CLIENTS: %d\n", *number_of_to_clients);


  } else if (flag == CLOSE_CLIENT) {
    // closes the client (both the to and from client descriptors) and downticks
    // the other trackers
    printf("[ " HYEL "SERVER" reset " ]: Client " HRED "DISCONNECT" reset "\n");
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
  } else if (flag == SEND_MESSAGE) {
    // TODO: What channel the message is being sent to... and check if subscribed to channels have updates

    char message[MESSAGE_SIZE];
    int x = read(*from_client, message, sizeof(message));
    strcat(chatHistory, message);
    strcat(chatHistory, "\n ");
    strcat(chatHistory, " ");
    // send the chat history to the client
    if (x > 0) {
      printf("[" HMAG " SERVER " reset "]: Client sent a message: %s!\n",
             message);
      //   TODO: send acknowledge message?
      for (int current_client_index = 0;
           current_client_index < *number_of_to_clients;
           current_client_index++) {
        // sends a random number to the clients, which is read by them and
        // printed out
        if (FD_ISSET(to_client_list[current_client_index],
                     &fd_set_of_to_client)) {
          int flag = SEND_MESSAGE;
          if (write(to_client_list[current_client_index], &flag,
                    sizeof(flag)) == -1)
            err();

          if (write(to_client_list[current_client_index], &chatHistory,
                    sizeof(chatHistory)) == -1)
            err();
        }
      }
      printf("[" HMAG " SERVER " reset "]: Sent message to all clients\n");
    } else if (x <= 0) {
      printf("Error reading message, client disconnected.\n");
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
  else if(flag == CREATE_CHANNEL){
    number_of_channels++;

    char channelName[MESSAGE_SIZE];
    int x = read(*from_client, channelName, sizeof(channelName));
    chatHistories[number_of_channels - 1] = (char *)calloc(MAX_SIZE_CHANNEL_NAME, sizeof(char));
    strcpy(channelNames[number_of_channels - 1], channelName);
    // LINE TO CHANGE CURRENT CHANNEL OF THIS CLIENT TO BE THE ONE THEY CREATED???


    // char * chatHistories[MAX_NUM_CHANNELS];
    // int currChannels[MAX_NUM_CLIENTS];
    // char * channelNames[MAX_NUM_CHANNELS];
  }
  else if(flag == CHANGE_CHANNEL){
    char channelName[MESSAGE_SIZE];
    int x = read(*from_client, channelName, sizeof(channelName));

    int channel_index = 0;
    while(strcmp(channelNames[channel_index], channelName) == 0){
      channel_index++;
    }

    // currChannels[this client] = channel_index;
    if (write(*to_client, chatHistories[channel_index], MAX_CHAT) == -1) err();

  }
  else if(flag == CLOSE_CHANNEL){

  }
  // add the nonblock
  fcntl(*from_client, F_SETFL, fcntl(*from_client, F_GETFL) | O_NONBLOCK);
}

/*=========================
  handle_sigpipe
  args: int sig

  handles and prints a notifying message if it catches a SIGPIPE signal,
  usually from a child disconnect

  returns ABSOLUTELY NOTHING
  =========================*/
void handle_sigpipe(int sig) {
  printf("[ " HRED "SERVER" reset " ]: Caught " HRED "SIGPIPE" reset
         ", client disconnected\n");
}

/*=========================
  handle_sigint
  args: int sig

  handles and prints a notifying message if it catches a SIGINT signal,
  usually from server disconnect, closes everything and then waits for a bit of
  time to make sure that the pipes work (an issue I had earlier)

  returns ABSOLUTELY NOTHING
  =========================*/
void handle_sigint(int sig) {
  printf("[ " HRED "SERVER" reset
         " ]: Caught SIGINT, closing server, number of clients to close: %d\n",
         number_of_to_clients);
  for (int i = 0; i < number_of_to_clients; i++) {
    int close_server_flag = CLOSE_SERVER;  // server flag defined in universal.h

    printf("[ " HRED "SERVER" reset " ]: Closing client %d\n", i);
    fcntl(to_client_list[i], F_SETFL,
          fcntl(to_client_list[i], F_GETFL) & ~O_NONBLOCK);

    if (write(to_client_list[i], &close_server_flag,
              sizeof(close_server_flag)) == -1) {
      printf("[ " HRED "SERVER" reset " ]: Error closing client %d\n", i);
      err();
    }
  }
  sleep(1);  // give clients a chance to read˝
  for (int i = 0; i < number_of_to_clients; i++) {
    close(to_client_list[i]);
    close(from_client_list[i]);
  }
  close(from_client_list[number_of_from_clients - 1]);
  printf("[ " HRED "SERVER" reset " ]: Caught " HRED "SIGINT" reset
         ", server disconnected\n");
  if (unlink(WKP) == -1) err();
  exit(0);
}
