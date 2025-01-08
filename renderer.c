#include <locale.h>
#include <ncurses.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

// gcc sigma.c -o sigma -lncurses
#include <curses.h>

#include "renderer.h"

#define MAX_INPUT 100
#define MAX_CHAT 30000

char input[MAX_INPUT] = {0};  // Input buffer
char chat[MAX_CHAT] = "READY PLAYER 0\n";
int ready_player_num = 0;

WINDOW *input_win, *chat_win;
int LINES, COLS, chat_open = 1;  // Default window size

int main() {
  setlocale(LC_ALL, "");
  initscr();  // Initialize ncurses

 // COLORS
  start_color();
  init_pair(1, COLOR_RED, COLOR_GREEN);   // chat box
  init_pair(2, COLOR_CYAN, COLOR_BLACK);  // prompt

  noecho();  // Disable automatic echoing of typed characters
  cbreak();  // Disable line buffering

  signal(SIGWINCH, handle_resize);
  signal(SIGINT, handle_sigint);
  int pos = 0;  // Current cursor position

  keypad(stdscr, TRUE);  // Enable function keys
  getmaxyx(stdscr, LINES, COLS);

  input_win = newwin(3, COLS, LINES - 3, 0);  // Create input window
  box(input_win, 0, 0);                       // Draw border around input window

  chat_win = newwin(LINES - 4, COLS, 1, 0);
  scrollok(chat_win, TRUE);
  attron(COLOR_PAIR(1));
  box(chat_win, 0, 0);  // Draw border around input window
  attroff(COLOR_PAIR(2));
  char *prompt = "CHAT WINDOW";
  attron(COLOR_PAIR(2));
  mvprintw(0, (COLS - strlen(prompt)) / 2, prompt);
  attroff(COLOR_PAIR(1));
  refresh();  // Refresh main screen

  mvwprintw(chat_win, 2, 3, "%s", chat);
  mvwprintw(input_win, 1, 1, "%s", input);  // Display input buffer

  while (1) {
    wrefresh(chat_win);
    wrefresh(input_win);

    int ch = getch();  // Capture user input

    if (ch == '\n') {
      handle_input(&chat_open);
      break;
    } else if ((ch == KEY_BACKSPACE || ch == 127) && pos > 0) {
      handle_backspace(input_win, chat_win, &pos, input);
    } else if (pos < MAX_INPUT - 1 && ch >= 32 && ch <= 126) {
      handle_normal_characters(input, &pos, ch);
      delwin(chat_win);
      chat_win = newwin(LINES - 4, COLS, 1, 0);
      scrollok(chat_win, TRUE);

      char chat_add[20];
      sprintf(chat_add, "  READY PLAYER %d\n", ready_player_num);
      ready_player_num++;
      strcat(chat, chat_add);
      mvwprintw(chat_win, 2, 2, "%s", chat);
      attron(COLOR_PAIR(1));
      box(chat_win, 0, 0);  // Draw border around input window
      attroff(COLOR_PAIR(1));
      wrefresh(chat_win);
      refresh();
    }
  }

  // Final output after exiting input
  clear();
  mvprintw(0, 0, "You entered: %s", input);
  refresh();

  endwin();  // Close ncurses mode
  return 0;
}

void handle_input(int *chat_open) { *chat_open = -1; }

void handle_backspace(WINDOW *input_win, WINDOW *chat_win, int *pos,
                      char *input) {
  (*pos)--;              // Move cursor back
  input[(*pos)] = '\0';  // Remove last character

  // Clear and redraw input window
  werase(input_win);
  box(input_win, 0, 0);
  mvwprintw(input_win, 1, 1, "%s", input);  // Redraw input text
  wrefresh(input_win);
}

void handle_normal_characters(char *input, int *pos, char ch) {
  input[(*pos)++] = ch;  // Append character
  input[(*pos)] = '\0';  // Null-terminate string

  mvwprintw(input_win, 1, 1, "%s", input);  // Update displayed text
  wrefresh(input_win);
}

void handle_resize(int sig) {
  if (chat_open == 1) {
    endwin();
    refresh();
    clear();

    getmaxyx(stdscr, LINES, COLS);

    attroff(COLOR_PAIR(2));
    char *prompt = "CHAT WINDOW";
    attron(COLOR_PAIR(2));
    mvprintw(0, (COLS - strlen(prompt)) / 2, prompt);
    refresh();

    delwin(chat_win);
    chat_win = newwin(LINES - 4, COLS, 1, 0);
    scrollok(chat_win, TRUE);
    mvwprintw(chat_win, 2, 3, "%s", chat);
    attron(COLOR_PAIR(1));
    box(chat_win, 0, 0);  // Draw border around input window}
    attroff(COLOR_PAIR(1));
    wrefresh(chat_win);

    delwin(input_win);
    input_win = newwin(3, COLS, LINES - 3, 0);  // Create input window
    box(input_win, 0, 0);  // Draw border around input window
    mvwprintw(input_win, 1, 1, "%s", input);  // Display input buffer
    wrefresh(input_win);
  }
}

void handle_sigint(int sig) {
  endwin();
  exit(0);
}