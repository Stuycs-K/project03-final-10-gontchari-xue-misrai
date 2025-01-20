# Dev Log:

This document must be updated daily by EACH group member.

## Elias Xu

### 2025-01-06

- Added proposal to the repo (created with Ivan earlier on Google Docs)
- Make the individial files and the makefile targets
- Created a basic renderer in renderer.c and simplified placement through renderer.h

## 2025-01-08

- Updated the client so that the frontend scrolls and also added *COLORS*.
- Made sure input worked in the chatbox

## 2025-01-09

- Add documentation for renderer and make a pr


## 2025-01-10 to 2025-01-12
- Reworked structure of chat app to use selecting
- Made it handle errors and other disconnects   

## 2025-01-13
- add comments
- refactor code

## 2025-01-14
- TODO: change the text box so that it increases sizes with input
- Did not do TODO, but I did start excalidraw drawing on protocols and deleted all of Ivan's comments

## 2025-01-15
- Created renderer example so that gathering of characters can be nonblocking
- Work with groupmates and teachers to brainstorm ways for ncurses to work without waiting for user input.

## 2025-01-16 - ___
- Worked (pair partnering) with Ivan on working on the server stuff for phase 2

## 2025-01-17 to 2025-01-19 - ___
- Finished the initial chat stuff, added boxes, fixed bugs
- Make it so that sizing works
- Make sure the text continually updates
- Edited out unessary code


## 2025-01-20 - ___
- ___
- ___
- ___





## Ivan Gontchar

### 2025-01-06 - Setup
- Created and worked on universal.h file for the project
- General setup and cleaning of initial project elements

### 2025-01-07 - Client server interaction
- Worked on implementing client, server, relationship code to establish a handshake/connection
- For now it's just functionally pretty much the same as the lab, and will be further customized as needed

## 2025-01-08 - Documentation points
- Added documentation for basically all functions in all .c files we have
- Updated Devlog for all future dates

## 2025-01-09 - General review and documentation day
- README draft in progress
- reviewed with elias, his rendering code and how it worked

## 2025-01-10 to 2025-01-12 - Cleaning up readme
- Finished README first draft that we can keep for now

## 2025-01-13 - Planning for channels which will be a lot
- Reviewed and read through all the code elias wrote, for clarity. Trying to best understand the logic so I can work with it.
- Wrote my implementation of the server-end recieving client messages and processing them
- I need to talk with my group mates about log history and other messaging logistics so when can deal with that.

## 2025-01-14 - Messaging between clients omg
- Cleaned up server side messaging setup
- Server writes chat history to a client, and incoming client messages are appended to chatHistory
- Server side messaging done i think

## 2025-01-15 - Functionalities and niceties
- Discussed and planned in class how to use ncurses and other functionalities to load user input and chat updates at the same time.
- Wrote code that generates a signature for the start of every message that looks like "username@pid"
- idk why but client.c has almost nothing, so I can't really add the signature to user messages, but that's quick so we'll see tomorrow

## 2025-01-16 - Absent
- Looking at the image Elias made for clarity of channel code to be written
- Trying to create a general idea of how to best approach this portion of the coding

## 2025-01-17 to 2025-01-19 - Channels
- Set up code in server.c that works with the different channel flags and what they should do
- Did code in client.c that can parse if a client is trying to execute a "/" command and which one
- Accompanying bug fixing and general cleaning to help make channels work
- Made channels work with user input and rendered chatbox
- Parse user args and redirecting to channel commands
- All channel commands work with user input
- All that's left is to display channels in the chatbox

## 2025-01-20 - final day wrapping up everything
- kept working on all channel functionality in general, making it work with user interface
- errors in chat if someone misuses a command
- overall cleaning up code and diagnostic prints
- everything channel related now seems to work on my end, so i am really happy!!!




## Ishaana Misra

### 2025-01-6
- start server struct

## 2025-01-08 - updating server struct
- have server create semaphore

## 2025-01-09 - shared memory
- have server create shared memory

## 2025-01-10 to 2025-01-12 - documentation
- we worked on code structure and worked on documentation

## 2025-01-13 - ___
- ___
- ___
- ___

## 2025-01-14 - client and server
- removed random numbers from client and server
- send chat history to clients upon connection

## 2025-01-15 - more working on client
- have client send first message to server
- we figured out the structure of getting client input

## 2025-01-16 - ___
- ___
- ___
- ___

## 2025-01-17 to 2025-01-19 - ___
- ___
- ___
- ___

## 2025-01-20 - ___
- ___
- ___
- ___
