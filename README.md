[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/Vh67aNdh)

# Discordia

### Got Is-Xus [28]

Ivan Gontchar, Elias Xu, Ishaana Misra

### Project Description:

We are going to simulate a chat app. Users will be able to type and read messages in different channels, which can be read by other users in those channels.

### Instructions:

How does the user install/compile/run the program?
- In the repo, any user should run the command "make compile".
- After this, the first user should run the command "./serv", which will launch the server end of the program.
 - Subsequent users who are running the program in other terminals should run the command "./cli" which will launch the client end of the program.
- This will cause the app to launch in the terminal, and thus the user should follow on-screen prompts.

How does the user interact with this program?
- The user will be put into the home, where they will have a list of channels they may enter. They then can type a prompt to enter a specific channel.
- Once they choose a channel, they will be able to see a few lines of previous activity, and then they will be able to send a message in it.
- Once sent, the message will show up in the channel and others who have access to the channel can see the message and respond accordingly.
- A user can always type “exit” to return to the list of all channels and enter a different one.
- The user can always type “quit” to exit the app entirely or use ctrl-C, which has error handling that helps with exiting gracefully.
