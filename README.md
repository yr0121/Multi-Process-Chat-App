The purpose of the program is to build a local multiprocess chat app that enables servers and up to 10 users to communicate.
To compile the project, just type make in the terminal
1) First, start the server program by type ./server and server window will show up.
2) In order to a create user,open another terminal window and locate to the same directory. Then type ./client <user name> in the terminal.
3) If want to add more users, just follow step (2).

The program supports several functions.
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
