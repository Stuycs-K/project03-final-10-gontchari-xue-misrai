#ifndef SERVER_H
#define SERVER_H
#include "universal.h"
void handle_sigpipe(int sig);
void handle_sigint(int sig);
void reset_fd_sets(fd_set *fd_set_of_from_client, fd_set *fd_set_of_to_client,
                   int number_of_to_clients, int number_of_from_clients,
                   int to_client_list[MAX_NUM_CLIENTS],
                   int from_client_list[MAX_NUM_CLIENTS], int *max_fd);
void handle_from_client(int *from_client, int *to_client, int *index,
                        int *to_client_list, int *from_client_list,
                        int *number_of_to_clients, int *number_of_from_clients,
                        int *new_number_of_from_clients, int *max_fd);
char * getChannelString(int index);
#endif
