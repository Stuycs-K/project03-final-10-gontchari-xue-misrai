[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/Vh67aNdh)

# Discordia

## Go(n)t Is-Xus? [28]

Ivan Gontchar, Elias Xu, Ishaana Misra

## Project Description:

We are going to simulate a chat app using pipes. Users will be able to type and read messages in different channels, which can be read by other users in those channels. Additionally, they'll know who else is in the "chat room." The window will be constantly resizng, and if it becomes too small the client will complain and force the user to resize. The chats will be managed by a server.

## Instructions:

### How does the user install/compile/run the program?

1. Clone the repo, and then enter the folder containing it:

```shell
git clone https://github.com/Stuycs-K/project03-final-10-gontchari-xue-misrai.git
cd project03-final-10-gontchari-xue-misrai
```

2. Premptively clean the folder, and then run the server:

```shell
make clean
make server # create the executables and the server executable
```

    They could also run:

```shell
make clean
make # create the executables
./serv # running the server executable
```

3. In any other terminals (and even across different usernames as long as they are on the same filesystem), run the client:

```shell
cd path/to/project03-final-10-gontchari-xue-misrai
make client # you can also run ./cli
```

4. Running make client will cause the chat to launch, and can type stuff into the input window and then send it through pressing enter. Additionally, they could use differnet commands to change or edit channels.

   - the `/create <channel_name>` command will create a channel with `channel_name`.
   - the `/remove <channel_name>` command will remove a channel with `channel_name`, unless it's the general channel. If so, that command will not work.
   - the `/switch <channel_name>` command will switch the user to a channel with the name specified.

5. To exit, the user can run `CTRL-C`, which will gracefully exit the program.

### How does the user interact with this program?

1. The user will enter onto the `general` channel.

2. Running make client will cause the chat to launch, and can type stuff into the input window and then send it through pressing enter. Additionally, they could use differnet commands to change or edit channels.

   - the `/create <channel_name>` command will create a channel with `channel_name`.
   - the `/remove <channel_name>` command will remove a channel with `channel_name`, unless it's the general channel. If so, that command will not work.
   - the `/switch <channel_name>` command will switch the user to a channel with the name specified.

3. Users can resize their screens at will, but if that resizing is too small, the chat will disappear and ask the user to make their screen bigger again.

4. To clear all text, the user can press `ESC`, which will clear all text on their screen.

5. To exit, the user can run `CTRL-C`, which will gracefully exit the program.

### External packages

Through testing on the stuy computers, there does not seem to be an external packages that need to be installed. Here is a list of the used packages (though not all are actually used in the code): 
    - `signal.h`
    - `sys/select.h`
    - `dirent.h`
    - `errno.h`
    - `fcntl.h`
    - `stdlib.h`
    - `string.h`
    - `sys/stat.h`
    - `sys/types.h`
    - `time.h`
    - `unistd.h`
    - `pwd.h`
    - `unistd.h`
    - `ncurses.h`
    - `locale.h`
    - `curses.h`

