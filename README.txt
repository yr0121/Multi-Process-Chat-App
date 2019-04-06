CSci4061 F2018 Assignment 2
* section: group number project2_66
* date: 11/09/18
* name: Yuanhao Ruan, Yiping Ren,Zhenyu Fan
* id: 5085043,5070041,5295805

1.The purpose of the program is to build a local multiprocess chat app that enables servers and up to 10 users to communicate.
2.Yuanhao Ruan wrote most part of the program, Yiping Ren and Zhenyu Fan helped debugging and testing
3.To compile the project, just type make in the terminal
4.(1)First, start the server program by type ./server and server window will show up.
  (2)In order to a create user,open another terminal window and locate to the same directory. Then type ./client <user name> in the terminal.
  (3)If want to add more users, just follow step (2).
5.The program supports several functions.
  Server: In the server window, you can type commands.
  (1)\list:list all the users, if no users right just show <no user>.
  (2)\kick <username>: kick a specific user out, terminate it and clean all its child process.
  (3)\exit: kick all the users, terminate all the users,and terminate server itself.
  (4)<any-other-text>, send all the active users with the form "Notice: <any-other-text>".
  (5)If press ctrl-c, kick all the users, terminate all the users,and terminate server itself.
  User:
  (1)\list:print active user list.
  (2)\p2p <username> <message>: send the designated user with message.
  (3)\exit: terminate itself from server without interrupting other users and server.
  (4)<any-other-text>: send information to all active users.
  (5)\seg: the system will print out segmentation fault error and terminate user without interrupting other users and server.
  (6)If press ctrl-c, the user will terminate without interrupting other uses and server.
6. We made assumptions that the user id length will not exceed MAX_USER_ID, the message legnth send between server and users
   will not exceed MAX_MSG.Unless specified, all the types message will broadcast to users.
7. With error handling, we closed proper pipe ends. In addition, we use perror to handle possible errors in different system
   calls and library functions.If such error occurs, we choose to exit the program. If the user limit is reach, the next added
   user will be terminated and we will print error message on the terminal. If users connect with a occupied user name, we will
   terminate the later user and print error message on the terminal.

