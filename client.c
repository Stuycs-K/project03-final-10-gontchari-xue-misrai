#include "client.h"

#include <curses.h>
#include <locale.h>
#include <ncurses.h>
#include <pwd.h>  // for struct passwd
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // for memset()
#include <string.h>
#include <sys/select.h>
#include <unistd.h>  // for usleep()

#include "colors.h"
#include "universal.h"

// just to head off any strcat issues
char chat[MAX_CHAT] = {0};
char signature[256] = {0};
char header_signature[256] = {0};
char header[512] = {0};
char *terminal_resize_prompt = "Resize your terminal";
char buffer[MESSAGE_SIZE - 256 - 1] = {0};
char displayed_buffer[MESSAGE_SIZE - 256 - 1] = {0};
int idx = 0;
int ch;
int col_shift = 4;

int min_height = 30, min_width = 60;
int chat_open = 1;
WINDOW *win_input, *win_chat, *win_people, *win_channel;
int ROWS, COLS;

fd_set to_server_fd_set, from_server_fd_set;
int from_server, to_server;

int boosted_input_height = 0;

int main() {
  setlocale(LC_ALL, "");

  char *usrnme;

  uid_t x = getuid();
  struct passwd *y = getpwuid(x);
  usrnme = y->pw_name;

  //   getting the name
  sprintf(signature, "%s@%d: ", usrnme, getpid());
  sprintf(header_signature, "%s@%d", usrnme, getpid());
  sprintf(header, "Your name is: %s", header_signature);

  from_server = client_handshake(&to_server);
  printf("[ " HCYN "CLIENT" reset " ]: Client side done\n");

  if (read(from_server, &chat, sizeof(chat)) == -1) err();

  FD_ZERO(&to_server_fd_set);
  FD_ZERO(&from_server_fd_set);
  FD_SET(from_server, &from_server_fd_set);
  FD_SET(to_server, &to_server_fd_set);

  signal(SIGWINCH, handle_resize);
  signal(SIGINT, handle_sigint);

  initscr();       // Start curses mode
  cbreak();        // Disable line buffering
  noecho();        // Don't echo() while we do getch
  curs_set(TRUE);  // Show the cursor (optional)

  start_color();
  init_pair(1, COLOR_MAGENTA, COLOR_BLACK);  // chat box
  init_pair(2, COLOR_CYAN, COLOR_BLACK);     // input box
  init_pair(3, COLOR_YELLOW, COLOR_BLACK);   // header
  init_pair(4, COLOR_GREEN, COLOR_BLACK);    // channel
  init_pair(5, COLOR_RED, COLOR_BLACK);      // people

  getmaxyx(stdscr, ROWS, COLS);

  // Create two windows
  //   the arguements are height, width, starty, startx
  win_chat = newwin(ROWS - 4, 3 * COLS / 4 + 1, 1, COLS / 4);
  win_channel = newwin((ROWS - 4) / 2, COLS / 4 - 1, 1, 0);
  win_people = newwin((ROWS - 4) / 2 + 1, COLS / 4 - 1, (ROWS - 4) / 2, 0);
  win_input = newwin(3, COLS, ROWS - 3, 0);
  // Draw initial boxes
  mvprintw(0, (COLS - strlen(header)) / 2, "%s", header);

  box(win_channel, 0, 0);
  wattron(win_channel, A_BOLD);
  wattron(win_channel, COLOR_PAIR(4));
  mvwprintw(win_channel, 0, 1, " Channels ");
  wattroff(win_channel, COLOR_PAIR(4));
  wattroff(win_channel, A_BOLD);
  wrefresh(win_channel);

  box(win_people, 0, 0);
  wattron(win_people, A_BOLD);
  wattron(win_people, COLOR_PAIR(5));
  mvwprintw(win_people, 0, 1, " People ");
  wattroff(win_people, COLOR_PAIR(5));
  wattroff(win_people, A_BOLD);
  wrefresh(win_people);

  box(win_chat, 0, 0);
  wattron(win_chat, A_BOLD);
  wattron(win_chat, COLOR_PAIR(1));
  mvwprintw(win_chat, 0, 1, " Chat ");
  wattroff(win_chat, COLOR_PAIR(1));
  wattroff(win_chat, A_BOLD);
  wrefresh(win_chat);

  box(win_input, 0, 0);
  wattron(win_input, A_BOLD);
  wattron(win_input, COLOR_PAIR(2));
  mvwprintw(win_input, 0, 1, " Input (ESC to clear) ");
  wattroff(win_input, COLOR_PAIR(2));
  wattroff(win_input, A_BOLD);
  wrefresh(win_input);

  // Move the cursor to the input window
  wmove(win_input, 1, 1 + strlen(displayed_buffer));

  // Make the input window non-blocking: wgetch() returns ERR if no input
  nodelay(win_input, TRUE);

  scrollok(win_chat, TRUE);
  scrollok(win_people, TRUE);
  scrollok(win_channel, TRUE);
  scrollok(win_input, TRUE);

  // Main loop
  while (1) {
    getmaxyx(stdscr, ROWS, COLS);
    if (ROWS < min_height || COLS < min_width) {
      chat_open = 0;
    } else {
      chat_open = 1;
    }

    if (chat_open == 1) {
      // refresh the FD_SETS
      FD_ZERO(&to_server_fd_set);
      FD_ZERO(&from_server_fd_set);
      FD_SET(from_server, &from_server_fd_set);
      FD_SET(to_server, &to_server_fd_set);

      int ret = select((to_server > from_server ? to_server : from_server) + 1,
                       &from_server_fd_set, &to_server_fd_set, NULL, NULL);

      if (ret == -1) {
        err();
      }

      if (FD_ISSET(from_server, &from_server_fd_set)) {
        int flag = 0;
        if (read(from_server, &flag, sizeof(flag)) == -1) err();
        if (flag == SEND_MESSAGE) {
          char new_chat[MAX_CHAT];
          if (read(from_server, new_chat, sizeof(new_chat)) == -1) err();
          // set the chat to the new chat
          chat[0] = 0;
          strcpy(chat, new_chat);
        } else if (flag == CLOSE_SERVER) {
          endwin();
          printf("[ " HCYN "CLIENT" reset " ]: Detected pipe " HRED
                 "CLOSURE" reset " by server; closing down\n");
          close(to_server);
          close(from_server);
          exit(0);
        } else {
          endwin();
          printf("Recieved Unknown flag: %d\n", flag);
          close(to_server);
          close(from_server);
          exit(0);
        }
      }

      refresh();
      // 3) Continuously update the other box
      // ! Order matters here! the rendering is very skibidi in this area
      attron(COLOR_PAIR(3));
      mvprintw(0, (COLS - strlen(header)) / 2, "%s", header);
      attroff(COLOR_PAIR(3));

      wresize(win_channel, (ROWS - 4) / 2, COLS / 4 - 1);
      mvwin(win_channel, 1, 0);
      werase(win_channel);
      box(win_channel, 0, 0);
      wattron(win_channel, A_BOLD);
      wattron(win_channel, COLOR_PAIR(4));
      mvwprintw(win_channel, 0, 1, " Channels ");
      wattroff(win_channel, COLOR_PAIR(4));
      wattroff(win_channel, A_BOLD);
      wrefresh(win_channel);

      wresize(win_people, (ROWS - 4) / 2, COLS / 4 - 1);
      mvwin(win_people, (ROWS - 4) / 2 + 1, 0);
      werase(win_people);
      box(win_people, 0, 0);
      wattron(win_people, A_BOLD);
      wattron(win_people, COLOR_PAIR(5));
      mvwprintw(win_people, 0, 1, " People ");
      wattroff(win_people, COLOR_PAIR(5));
      wattroff(win_people, A_BOLD);
      wrefresh(win_people);

      wresize(win_chat, ROWS - 4, 3 * COLS / 4 + 1);
      mvwin(win_chat, 1, COLS / 4);
      werase(win_chat);
      mvwprintw(win_chat, 1, 2, "%s", chat);
      box(win_chat, 0, 0);
      wattron(win_chat, A_BOLD);
      wattron(win_chat, COLOR_PAIR(1));
      mvwprintw(win_chat, 0, 1, " Chat ");
      wattroff(win_chat, COLOR_PAIR(1));
      wattroff(win_chat, A_BOLD);
      wrefresh(win_chat);

      // 2) Update the input window
      wresize(win_input, 3, COLS);
      mvwin(win_input, ROWS - 3, 0);
      werase(win_input);
      mvwprintw(win_input, 1, 1, "%s", displayed_buffer);
      box(win_input, 0, 0);
      wattron(win_input, A_BOLD);
      wattron(win_input, COLOR_PAIR(2));
      mvwprintw(win_input, 0, 2, " Input (ESC to clear) ");
      wattroff(win_input, COLOR_PAIR(2));
      wattroff(win_input, A_BOLD);
      wmove(win_input, 1, 1 + strlen(displayed_buffer));

      wrefresh(win_input);

      // poll for input from server
      static char
          int_buf[sizeof(int)];  // this is a static variable because we
                                 // want to keep the buffer between calls
      static int bytes_collected = 0;

      ch = wgetch(win_input);
      if (ch != ERR) {
        // Handle character
        if (ch == '\n') {
          // For example, you could "submit" the buffer here
          // We'll just clear it
          // TODO send message to server
          int flag = SEND_MESSAGE;
          char message[MESSAGE_SIZE] = {0};
          strcat(message, signature);
          strcat(message, buffer);
          if (write(to_server, &flag, sizeof(flag)) == -1) err();
          if (write(to_server, message, sizeof(message)) == -1) err();
          idx = 0;

          buffer[0] = 0;
          displayed_buffer[0] = 0;
        } else if (ch == 27) {
          // If ESC pressed, clear the buffer
          idx = 0;
          buffer[0] = '\0';
          displayed_buffer[0] = '\0';
        } else if (ch == KEY_BACKSPACE || ch == 127) {
          // Handle backspace
          if (idx > 0) {
            buffer[--idx] = '\0';
          }
          //   update the displayed buffer display the
          // last number of characters in the buffer equal to the number of
          // columns - col_shift
          displayed_buffer[0] = '\0';
          strcpy(displayed_buffer, buffer);
          if (strlen(displayed_buffer) > COLS - col_shift) {
            strncpy(displayed_buffer,
                    buffer + strlen(buffer) - (COLS - col_shift),
                    COLS - col_shift);
            displayed_buffer[COLS - col_shift] = '\0';
          }

        } else if (idx < (int)sizeof(buffer) - 1) {
          // Append typed character
          buffer[idx++] = ch;
          buffer[idx] = '\0';

          displayed_buffer[0] = '\0';
          strcpy(displayed_buffer, buffer);
          if (strlen(displayed_buffer) > COLS - col_shift) {
            strncpy(displayed_buffer,
                    buffer + strlen(buffer) - (COLS - col_shift),
                    COLS - col_shift);
            displayed_buffer[COLS - col_shift] = '\0';
          }
        }
      }
    } else {
      // for really small screens
      refresh();
      clear();

      mvprintw(ROWS / 2 - 1, (COLS - sizeof(terminal_resize_prompt) - 4) / 2,
               "%s", terminal_resize_prompt);

      mvprintw(ROWS / 2, (COLS / 2) - 5, "Width ");
      attron(A_BOLD);
      if (COLS < min_width) {
        attron(COLOR_PAIR(5));
      } else {
        attron(COLOR_PAIR(4));
      }
      mvprintw(ROWS / 2, (COLS / 2) + 5, "%d", COLS);
      if (COLS < min_width) {
        attroff(COLOR_PAIR(5));
      } else {
        attroff(COLOR_PAIR(4));
      }
      attroff(A_BOLD);

      mvprintw(ROWS / 2 + 1, (COLS / 2) - 5, "Height ");
      attron(A_BOLD);
      if (ROWS < min_height) {
        attron(COLOR_PAIR(5));
      } else {
        attron(COLOR_PAIR(4));
      }
      mvprintw(ROWS / 2 + 1, (COLS / 2) + 5, "%d", ROWS);
      if (ROWS < min_height) {
        attroff(COLOR_PAIR(5));
      } else {
        attroff(COLOR_PAIR(4));
      }
      attroff(A_BOLD);

      refresh();
    }

    // Small delay so we don’t hog the CPU
    usleep(1000);  // 0.1 seconds
  }

  // Cleanup
  delwin(win_chat);
  delwin(win_input);
  endwin();  // End curses mode

  return 0;
}

void handle_resize(int sig) {
  getmaxyx(stdscr, ROWS, COLS);

  if (ROWS < min_height || COLS < min_width) {
    chat_open = 0;
  } else {
    chat_open = 1;
  }

  if (strlen(displayed_buffer) > COLS - col_shift) {
    strncpy(displayed_buffer, buffer + strlen(buffer) - (COLS - col_shift),
            COLS - col_shift);
    displayed_buffer[COLS - col_shift] = '\0';
  }

  if (chat_open == 1) {
    endwin();
    refresh();
    clear();
    getmaxyx(stdscr, ROWS, COLS);

    if (strlen(displayed_buffer) > COLS - col_shift) {
      strncpy(displayed_buffer, buffer + strlen(buffer) - (COLS - col_shift),
              COLS - col_shift);
      displayed_buffer[COLS - col_shift] = '\0';
    }

    attron(COLOR_PAIR(3));
    mvprintw(0, (COLS - strlen(header)) / 2, "%s", header);
    attroff(COLOR_PAIR(3));

    wresize(win_channel, (ROWS - 4) / 2, COLS / 4);
    mvwin(win_channel, 1, 0);
    werase(win_channel);
    box(win_channel, 0, 0);
    wattron(win_channel, A_BOLD);
    wattron(win_channel, COLOR_PAIR(4));
    mvwprintw(win_channel, 0, 1, " Channels ");
    wattroff(win_channel, COLOR_PAIR(4));
    wattroff(win_channel, A_BOLD);
    wrefresh(win_channel);

    wresize(win_people, (ROWS - 4) / 2, COLS / 4);
    mvwin(win_people, (ROWS - 4) / 2 + 1, 0);
    werase(win_people);
    box(win_people, 0, 0);
    wattron(win_people, A_BOLD);
    wattron(win_people, COLOR_PAIR(5));
    mvwprintw(win_people, 0, 1, " People ");
    wattroff(win_people, COLOR_PAIR(5));
    wattroff(win_people, A_BOLD);
    wrefresh(win_people);

    wresize(win_chat, ROWS - 4, 3 * COLS / 4 + 1);
    mvwin(win_chat, 1, COLS / 4);
    werase(win_chat);
    mvwprintw(win_chat, 1, 2, "%s", chat);
    box(win_chat, 0, 0);
    wattron(win_chat, A_BOLD);
    wattron(win_chat, COLOR_PAIR(1));
    mvwprintw(win_chat, 0, 1, " Chat ");
    wattroff(win_chat, COLOR_PAIR(1));
    wattroff(win_chat, A_BOLD);
    wrefresh(win_chat);

    // 2) Update the input window
    wresize(win_input, 3, COLS);
    mvwin(win_input, ROWS - 3, 0);
    werase(win_input);
    mvwprintw(win_input, 1, 1, "%s", displayed_buffer);
    box(win_input, 0, 0);
    wattron(win_input, A_BOLD);
    wattron(win_input, COLOR_PAIR(2));
    mvwprintw(win_input, 0, 2, " Input (ESC to clear) ");
    wattroff(win_input, COLOR_PAIR(2));
    wattroff(win_input, A_BOLD);
    wmove(win_input, 1, 1 + strlen(displayed_buffer));
    wrefresh(win_input);

    refresh();
    scrollok(win_chat, TRUE);
    scrollok(win_people, TRUE);
    scrollok(win_channel, TRUE);
    scrollok(win_input, TRUE);
  } else if (chat_open == 0) {
    endwin();
    refresh();
    clear();
    getmaxyx(stdscr, ROWS, COLS);
    mvprintw(ROWS / 2 - 1, (COLS - sizeof(terminal_resize_prompt) - 4) / 2,
             "%s", terminal_resize_prompt);
    mvprintw(ROWS / 2, (COLS / 2) - 5, "Width ");

    attron(A_BOLD);
    if (COLS < min_width) {
      attron(COLOR_PAIR(5));
    } else {
      attron(COLOR_PAIR(4));
    }
    mvprintw(ROWS / 2, (COLS / 2) + 5, "%d", COLS);
    if (COLS < min_width) {
      attroff(COLOR_PAIR(5));
    } else {
      attroff(COLOR_PAIR(4));
    }
    attroff(A_BOLD);

    mvprintw(ROWS / 2 + 1, (COLS / 2) - 5, "Height ");
    attron(A_BOLD);
    if (ROWS < min_height) {
      attron(COLOR_PAIR(5));
    } else {
      attron(COLOR_PAIR(4));
    }
    mvprintw(ROWS / 2 + 1, (COLS / 2) + 5, "%d", ROWS);
    if (ROWS < min_height) {
      attroff(COLOR_PAIR(5));
    } else {
      attroff(COLOR_PAIR(4));
    }
    attroff(A_BOLD);

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
