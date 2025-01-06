#ifndef RENDERER_H
#define RENDERER_H
#include <ncurses.h>
void handle_input(int *chat_open);
void handle_backspace(WINDOW *input_win, WINDOW *chat_win, int *pos,
                      char *input);
void handle_normal_characters(char *input, int *pos, char ch);
void handle_resize(int sig);
void handle_sigint(int sig);
#endif