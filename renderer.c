#include <locale.h>
#include <ncurses.h>
#include <signal.h>
#include <string.h>

// gcc sigma.c -o sigma -lncurses
#include "renderer.h"

#define MAX_INPUT 50

char input[MAX_INPUT] = {0};  // Input buffer
WINDOW *input_win, *chat_win;
int LINES, COLS, chat_open = 1;  // Default window size

int main() {
  initscr();  // Initialize ncurses
  noecho();   // Disable automatic echoing of typed characters
  cbreak();   // Disable line buffering

  keypad(stdscr, TRUE);  // Enable function keys
  getmaxyx(stdscr, LINES, COLS);
  signal(SIGWINCH, handle_resize);
  signal(SIGINT, handle_sigint);
  int pos = 0;  // Current cursor position

  input_win = newwin(3, COLS, LINES - 3, 0);  // Create input window
  box(input_win, 0, 0);                       // Draw border around input window

  chat_win = newwin(LINES - 4, COLS, 1, 0);
  box(chat_win, 0, 0);  // Draw border around input window

  mvprintw(0, 0, "Type something (Backspace to delete, Enter to submit):");
  refresh();  // Refresh main screen

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
    }
  }

  // Final output after exiting input
  clear();
  mvprintw(0, 0, "You entered: %s", input);
  refresh();

  getch();   // Wait for key press before exiting
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

    mvprintw(0, 0, "Type something (Backspace to delete, Enter to submit):");
    refresh();

    delwin(chat_win);
    chat_win = newwin(LINES - 4, COLS, 1, 0);
    box(chat_win, 0, 0);  // Draw border around input window}
    wrefresh(chat_win);

    delwin(input_win);
    input_win = newwin(3, COLS, LINES - 3, 0);  // Create input window
    box(input_win, 0, 0);  // Draw border around input window
    wrefresh(input_win);
  }
}

void handle_sigint(int sig) { endwin(); }
