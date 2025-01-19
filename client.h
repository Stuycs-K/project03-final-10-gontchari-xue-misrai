#ifndef CLIENT_H
#define CLIENT_H
void handle_sigint(int sig);
void handle_resize(int sig);
void render_channel_box();
void render_people_box();
void render_input_box();
void render_chat_box();
void update_cursor_position();
#endif