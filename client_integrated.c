#include <curses.h>
#include <locale.h>
#include <ncurses.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>  // for memset()
#include <unistd.h>  // for usleep()

#include "client.h"
#include "universal.h"

char chat[MAX_CHAT];

int chat_open = 1;

WINDOW *win_input, *win_update;

char buffer[128] = {0};
int ROWS, COLS;
int from_server, to_server;

int main() {
  setlocale(LC_ALL, "");

  signal(SIGWINCH, handle_resize);
  signal(SIGINT, handle_sigint);

  from_server = client_handshake(&to_server);
  printf("Got past client handshake\n");

  if (read(from_server, &chat, sizeof(chat)) == -1) err();

  initscr();       // Start curses mode
  cbreak();        // Disable line buffering
  noecho();        // Don't echo() while we do getch
  curs_set(TRUE);  // Show the cursor (optional)

  start_color();
  init_pair(1, COLOR_RED, COLOR_GREEN);   // chat box
  init_pair(2, COLOR_CYAN, COLOR_BLACK);  // prompt

  getmaxyx(stdscr, ROWS, COLS);

  // Create two windows
  WINDOW *win_input = newwin(3, COLS, ROWS - 3, 0);
  WINDOW *win_update = newwin(ROWS - 4, COLS, 1, 0);

  // Draw initial boxes

  box(win_update, 0, 0);
  mvwprintw(win_update, 0, 2, " Update ");
  wrefresh(win_update);

  box(win_input, 0, 0);
  mvwprintw(win_input, 0, 2, " Input ");
  wrefresh(win_input);

  // Make the input window non-blocking: wgetch() returns ERR if no input
  nodelay(win_input, TRUE);
  //   keypad(win_input, TRUE);  // Enable special keys (arROWS, etc.) if you
  //   want

  // Prepare a small buffer to hold typed input
  memset(buffer, 0, sizeof(buffer));
  int idx = 0;

  int ch;

  // Main loop
  while (1) {
    refresh();
    // 3) Continuously update the other box
    wresize(win_update, ROWS - 4, COLS);
    mvwin(win_update, 1, 0);
    werase(win_update);
    box(win_update, 0, 0);
    mvwprintw(win_update, 0, 2, " Update ");
    mvwprintw(win_update, 1, 2, "%s", chat, COLS, ROWS);
    wrefresh(win_update);

    // 2) Update the input window
    wresize(win_input, 3, COLS);
    mvwin(win_input, ROWS - 3, 0);
    werase(win_input);
    box(win_input, 0, 0);
    mvwprintw(win_input, 0, 2, " Input ");
    mvwprintw(win_input, 1, 1, "%s", buffer);
    wrefresh(win_input);

    // 1) Poll for input from win_input
    ch = wgetch(win_input);
    if (ch != ERR) {
      // Handle character
      if (ch == '\n') {
        // For example, you could "submit" the buffer here
        // We'll just clear it
        idx = 0;
        memset(buffer, 0, sizeof(buffer));
      } else if (ch == 27) {
        // If ESC pressed, break (exit)
        break;
      } else if (ch == KEY_BACKSPACE || ch == 127) {
        // Handle backspace
        if (idx > 0) {
          buffer[--idx] = '\0';
        }
      } else if (idx < (int)sizeof(buffer) - 1) {
        // Append typed character
        buffer[idx++] = ch;
        buffer[idx] = '\0';
      }
    }

    // Small delay so we don’t hog the CPU
    usleep(10000);  // 0.1 seconds
  }

  // Cleanup
  delwin(win_update);
  delwin(win_input);
  endwin();  // End curses mode

  return 0;
}

void handle_resize(int sig) {
  if (chat_open == 1) {
    endwin();
    refresh();
    clear();
    getmaxyx(stdscr, ROWS, COLS);
    resizeterm(ROWS, COLS);

    wresize(win_input, 3, COLS);
    mvwin(win_input, ROWS - 3, 0);
    werase(win_input);
    box(win_input, 0, 0);
    mvwprintw(win_input, 0, 2, " Input ");
    mvwprintw(win_input, 1, 1, "%s", buffer);
    wrefresh(win_input);

    wresize(win_update, ROWS - 4, COLS);
    mvwin(win_update, 1, 0);
    werase(win_update);
    box(win_update, 0, 0);
    mvwprintw(win_update, 0, 2, " Update ");
    mvwprintw(win_update, 1, 2, "%s", chat, COLS, ROWS);
    wrefresh(win_update);
    refresh();
  }
}

void handle_sigint(int sig) {
  int flag = CLOSE_CLIENT;
  if (write(to_server, &flag, sizeof(flag)) == -1) err();
  sleep(1);
  close(to_server);
  close(from_server);
  char fifo_name[PIPE_SIZING] = {"\0"};
  sprintf(fifo_name, "%d", getpid());
  char *fifo_ending = ".fifo";
  strcat(fifo_name, fifo_ending);
  unlink(fifo_name);

  endwin();
  exit(0);
}
