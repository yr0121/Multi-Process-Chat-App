/* CSci4061 F2018 Assignment 2
* section: group number project2_66
* date: 11/09/18
* name: Yuanhao Ruan, Yiping Ren,Zhenyu Fan
* id: 5085043,5070041,5295805 */
#include <stdio.h> 
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include "comm.h"
#include "util.h"
#include <signal.h>

/* -----------Functions that implement server functionality -------------------------*/

/*
 * Returns the empty slot on success, or -1 on failure
 */
int find_empty_slot(USER * user_list) {
	// iterate through the user_list and check m_status to see if any slot is EMPTY
	// return the index of the empty slot
    int i = 0;
	for(i=0;i<MAX_USER;i++) {
    	if(user_list[i].m_status == SLOT_EMPTY) {
			return i;
		}
	}
	return -1;
}

/*
 * list the existing users on the server shell
 */
int list_users(int idx, USER * user_list)
{
	// iterate through the user list
	// if you find any slot which is not empty, print that m_user_id
	// if every slot is empty, print "<no users>""
	// If the function is called by the server (that is, idx is -1), then printf the list
	// If the function is called by the user, then send the list to the user using write() and passing m_fd_to_user
	// return 0 on success
	int i, flag = 0;
	char buf[MAX_MSG] = {}, *s = NULL;

	/* construct a list of user names */
	s = buf;
	strncpy(s, "---connecetd user list---\n", strlen("---connecetd user list---\n"));
	s += strlen("---connecetd user list---\n");
	for (i = 0; i < MAX_USER; i++) {
		if (user_list[i].m_status == SLOT_EMPTY)
			continue;
		flag = 1;
		strncpy(s, user_list[i].m_user_id, strlen(user_list[i].m_user_id));
		s = s + strlen(user_list[i].m_user_id);
		strncpy(s, "\n", 1);
		s++;
	}
	if (flag == 0) {
		strcpy(buf, "<no users>\n");
	} else {
		s--;
		strncpy(s, "\0", 1);
	}

	if(idx < 0) {
		printf(buf);
		printf("\n");
	} else {
		/* write to the given pipe fd */
		if (write(user_list[idx].m_fd_to_user, buf, strlen(buf) + 1) < 0)
			perror("writing to server shell");
	}

	return 0;
}

/*
 * add a new user
 */
int add_user(int idx, USER * user_list, int pid, char * user_id, int pipe_to_child, int pipe_to_parent)
{
	// populate the user_list structure with the arguments passed to this function
	// return the index of user added
	user_list[idx].m_pid=pid;
	strcpy(user_list[idx].m_user_id,user_id);
	user_list[idx].m_fd_to_user=pipe_to_child;
	user_list[idx].m_fd_to_server=pipe_to_parent;
	user_list[idx].m_status=SLOT_FULL;
	return idx;
}

/*
 * Kill a user
 */
void kill_user(int idx, USER * user_list) {
	// kill a user (specified by idx) by using the systemcall kill()
	// then call waitpid on the user
	int pid=user_list[idx].m_pid;
	kill(pid,SIGKILL);
	waitpid(pid,NULL,0);
}

/*
 * Perform cleanup actions after the used has been killed
 */
void cleanup_user(int idx, USER * user_list)
{
	// m_pid should be set back to -1
	// m_user_id should be set to zero, using memset()
	// close all the fd
	// set the value of all fd back to -1
	// set the status back to empty
	user_list[idx].m_pid=-1;
	memset(user_list[idx].m_user_id,'\0',MAX_USER_ID);
	close(user_list[idx].m_fd_to_user);
	close(user_list[idx].m_fd_to_server);
	user_list[idx].m_fd_to_user=-1;
	user_list[idx].m_fd_to_server=-1;
	user_list[idx].m_status=SLOT_EMPTY;
}

/*
 * Kills the user and performs cleanup
 */
void kick_user(int idx, USER * user_list) {
	// should kill_user()
	// then perform cleanup_user()
	kill_user(idx,user_list);
	cleanup_user(idx,user_list);
}

/*
 * broadcast message to all users
 */
int broadcast_msg(USER * user_list, char *buf, char *sender)
{
	//iterate over the user_list and if a slot is full, and the user is not the sender itself,
	//then send the message to that user
	//return zero on success
	char message[MAX_MSG+strlen(sender)+strlen(": ")];
	strcpy(message,sender);
	strcat(message,": ");
	strcat(message,buf);
	int i;
	for(i=0;i<MAX_USER;i++) {
		if((user_list[i].m_status==SLOT_FULL) && (strcmp(sender,user_list[i].m_user_id)!=0)) {
			if(write(user_list[i].m_fd_to_user,message,strlen(message))<0) {
				perror("broadcast fails");
			}
		}
	}
	return 0;
}

/*
 * Cleanup user chat boxes
 */
void cleanup_users(USER * user_list)
{
	int i;
	for(i=0;i<MAX_USER;i++) {
		if(user_list[i].m_status==SLOT_EMPTY) {
			cleanup_user(i,user_list);
		}
	}
	// go over the user list and check for any empty slots
	// call cleanup user for each of those users.
}

/*
 * find user index for given user name
 */
int find_user_index(USER * user_list, char * user_id)
{
	// go over the  user list to return the index of the user which matches the argument user_id
	// return -1 if not found
	int i, user_idx = -1;

	if (user_id == NULL) {
		fprintf(stderr, "NULL name passed.\n");
		return user_idx;
	}
	for (i=0;i<MAX_USER;i++) {
		if (user_list[i].m_status == SLOT_EMPTY)
			continue;
		if (strcmp(user_list[i].m_user_id, user_id) == 0) {
			return i;
		}
	}

	return -1;
}

/*
 * given a command's input buffer, extract name
 */
int extract_name(char * buf, char * user_name)
{
	char inbuf[MAX_MSG];
    char * tokens[16];
    strcpy(inbuf, buf);

    int token_cnt = parse_line(inbuf, tokens, " ");

    if(token_cnt >= 2) {
        strcpy(user_name, tokens[1]);
        return 0;
    }

    return -1;
}

int extract_text(char *buf, char * text)
{
    char inbuf[MAX_MSG];
    char * tokens[16];
    char * s = NULL;
    strcpy(inbuf, buf);

    int token_cnt = parse_line(inbuf, tokens, " ");

    if(token_cnt >= 3) {
        //Find " "
        s = strchr(buf, ' ');
        s = strchr(s+1, ' ');

        strcpy(text, s+1);
        return 0;
    }

    return -1;
}

/*
 * send personal message
 */
void send_p2p_msg(int idx, USER * user_list, char *buf)
{
	// get the target user by name using extract_name() function
	// find the user id using find_user_index()
	// if user not found, write back to the original user "User not found", using the write()function on pipes. 
	// if the user is found then write the message that the user wants to send to that user.
	char target[MAX_USER_ID];
	int flag=extract_name(buf,target);
	if(flag==-1) {
		write(user_list[idx].m_fd_to_user,"User not found",strlen("User not found"));
	}else {
		int index=find_user_index(user_list,target);
		if(index==-1) {
			write(user_list[idx].m_fd_to_user,"User not found",strlen("User not found"));
		}else {
			char text[MAX_MSG+MAX_USER_ID+strlen(": ")];
			char info[MAX_MSG];
			strcpy(text,user_list[idx].m_user_id);
			strcat(text,": ");
			extract_text(buf,info);
			strcat(text,info);
			write(user_list[index].m_fd_to_user,text,strlen(text));
		}
	}
}

//takes in the filename of the file being executed, and prints an error message stating the commands and their usage
// void show_error_message(char *filename)
// {
// }


/*
 * Populates the user list initially
 */
void init_user_list(USER * user_list) {

	//iterate over the MAX_USER
	//memset() all m_user_id to zero
	//set all fd to -1
	//set the status to be EMPTY
	int i=0;
	for(i=0;i<MAX_USER;i++) {
		user_list[i].m_pid = -1;
		memset(user_list[i].m_user_id, '\0', MAX_USER_ID);
		user_list[i].m_fd_to_user = -1;
		user_list[i].m_fd_to_server = -1;
		user_list[i].m_status = SLOT_EMPTY;
	}
}



/* ---------------------End of the functions that implementServer functionality -----------------*/


/* ---------------------Start of the Main function ----------------------------------------------*/
int main(int argc, char * argv[])
{

	int nbytes;
	setup_connection("group66"); // Specifies the connection point as argument.

	USER user_list[MAX_USER];
	init_user_list(user_list);   // Initialize user list

	char buf[MAX_MSG]; 
	fcntl(0, F_SETFL, fcntl(0, F_GETFL)| O_NONBLOCK);
	print_prompt("admin");

	//
	while(1) {
		/* ------------------------YOUR CODE FOR MAIN--------------------------------*/

		// Handling a new connection using get_connection
		usleep(1000);
		//create necessary pipes
		int pipe_SERVER_reading_from_child[2];
		int pipe_SERVER_writing_to_child[2];
		char user_id[MAX_USER_ID];
		int pipe_child_writing_to_user[2];
		int pipe_child_reading_from_user[2];
		

		int connection=get_connection(user_id, pipe_child_writing_to_user, pipe_child_reading_from_user);
		//a flag to control if add users
		int flag=1;
		//if connect
		if(connection!=-1) {
			//when no space is available
			int index=find_empty_slot(user_list);
				if(index==-1) {
					perror("No space available,reach maximum user number limit\n");
					write(pipe_child_writing_to_user[1],"No_slot",strlen("No_slot"));
					// close(pipe_child_writing_to_user[1]);
					// close(pipe_child_reading_from_user[0]);
					print_prompt("admin");
					flag=0;
				}
				//when same user id
				if(find_user_index(user_list,user_id)!=-1) {
					write(pipe_child_writing_to_user[1],"User_name_already_used",strlen("User_name_already_used"));
					perror("User name already used");
					//kick_user(index,user_list);
					print_prompt("admin");
					flag=0;
				}
			//check pipe error
			if(pipe(pipe_SERVER_writing_to_child)<0) {
				perror("pipe failed\n");
				exit(-1);
			}

			if(pipe(pipe_SERVER_reading_from_child)<0) {
				perror("pipe failed\n");
				exit(-1);
			}
			//user connect and add
			if(flag==1) {
			int pid=fork();
			//child process dealing with server and users
			if(pid==0) {
				//close unnecessary pipes
				close(pipe_SERVER_reading_from_child[0]);
				close(pipe_SERVER_writing_to_child[1]);
				close(pipe_child_reading_from_user[1]);
				close(pipe_child_writing_to_user[0]);
				//non blocking setting
				fcntl(pipe_SERVER_writing_to_child[0],F_SETFL,fcntl(pipe_SERVER_writing_to_child[0],F_GETFL)|O_NONBLOCK);
				fcntl(pipe_child_reading_from_user[0],F_SETFL,fcntl(pipe_child_reading_from_user[0],F_GETFL)|O_NONBLOCK);
				while(1) {
					usleep(1000);
					memset(buf,'\0',MAX_MSG);
					int nbytes1=read(pipe_child_reading_from_user[0],buf,MAX_MSG);
					if(nbytes1!=-1) {
						//printf("buf is %s with length %d\n",buf,nbytes1 );
						if(nbytes1>0) {
							if(write(pipe_SERVER_reading_from_child[1],buf,strlen(buf))<0) {
								perror("write to server failed");
							}
							memset(buf,'\0', MAX_MSG);
						}
						//close pipes
						if(nbytes1==0) {
							close(pipe_child_reading_from_user[0]);
							close(pipe_child_reading_from_user[1]);
							close(pipe_child_writing_to_user[1]);
							close(pipe_child_writing_to_user[0]);
							close(pipe_SERVER_writing_to_child[0]);
							close(pipe_SERVER_reading_from_child[1]);
							exit(-1);
						}
					}

					int nbytes2=read(pipe_SERVER_writing_to_child[0],buf,MAX_MSG);
					if(nbytes2!=-1) {
						if(nbytes2>0) {
							if(write(pipe_child_writing_to_user[1],buf,strlen(buf))<0) {
								perror("write to user failed");
							}
							memset(buf,'\0', MAX_MSG);
						}
						//close pipes
						if(nbytes2==0) {
							close(pipe_SERVER_writing_to_child[0]);
							close(pipe_SERVER_reading_from_child[1]);
							close(pipe_child_writing_to_user[1]);
							close(pipe_child_writing_to_user[0]);
							close(pipe_child_reading_from_user[0]);
							close(pipe_child_reading_from_user[1]);
							exit(-1);
						}
					}

				}
				//parent process: close pipes and add users
			}else if(pid>0) {
				close(pipe_child_writing_to_user[1]);
				close(pipe_child_reading_from_user[0]);
				add_user(index,user_list,pid,user_id,pipe_SERVER_writing_to_child[1],pipe_SERVER_reading_from_child[0]);
				printf("user %s connects to server with slot %d\n", user_id,index);
				print_prompt("admin");

			}else {
				perror("fork failed\n");
				exit(-1);
			}
		}
	}
	
		//poll stdin from terminal

		int bytes=read(0,buf,MAX_MSG);
		if(bytes==1) {
			print_prompt("admin");
			continue;
		}
		if(bytes!=-1) {
			if(bytes>0) {
				//remove new line character
				buf[strlen(buf)-1]='\0';
				enum command_type command = get_command_type(buf);
				//check each command and do appropriate action
				switch(command) {
					case LIST:
						list_users(-1,user_list);
						break;
					case KICK: {
						//extract the name of the user and kick it
						char username[MAX_USER_ID];
						extract_name(buf,username);
						printf("The user to kick is %s\n", username);
						int idx=find_user_index(user_list,username);
						kick_user(idx,user_list);
						break;
					}
					case EXIT: {
						//kick all users in the list
						int i;
						for(i=0;i<MAX_USER;i++) {
							if(user_list[i].m_status==SLOT_FULL) {
								kick_user(i,user_list);
							}
						}
						exit(0);
						break;
					}
					default: {
						broadcast_msg(user_list,buf,"Notice");
					}
				}
			}
			memset(buf,'\0',MAX_MSG);
			print_prompt("admin");
		}

		// //poll users
		int i;
		for(i=0;i<MAX_USER;i++) {
			if(user_list[i].m_status==SLOT_FULL) {
				fcntl(user_list[i].m_fd_to_server,F_SETFL,fcntl(user_list[i].m_fd_to_server,F_GETFL)|O_NONBLOCK);
				int bytes=read(user_list[i].m_fd_to_server,buf,MAX_MSG);
				if(bytes!=-1) {
					//cases handling control c in user
					if(bytes>0) {
						if(strcmp("signal",buf)==0) {
							cleanup_user(i,user_list);
							break;
						}
						enum command_type command = get_command_type(buf);
						//dealing with different command
						switch(command) {
							case LIST:
								list_users(i,user_list);
								break;
							case P2P:
								send_p2p_msg(i,user_list,buf);
								break;
							case EXIT:
								printf("user %s exit\n",user_list[i].m_user_id);
								kick_user(i,user_list);
								print_prompt("admin");
								break;
							case SEG:
								 cleanup_user(i,user_list);
								 break;
							default:{
								broadcast_msg(user_list,buf,user_list[i].m_user_id);
							}
						}
						memset(buf,'\0',MAX_MSG);
					}
					if(bytes==0) {
						cleanup_user(i,user_list);
					}
				}
			}
		}
	}

}

		
	
		/* ------------------------YOUR CODE FOR MAIN--------------------------------*/
		

/* --------------------End of the main function ----------------------------------------*/
