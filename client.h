#ifndef CLIENT_H
#define CLIENT_H
void handle_sigint(int sig);
void parse_args( char * line, char ** arg_ary );
void handle_resize(int sig);
#endif
