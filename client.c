/* CSci4061 F2018 Assignment 2
* section: group number project2_66
* date: 11/09/18
* name: Yuanhao Ruan, Yiping Ren,Zhenyu Fan
* id: 5085043,5070041,5295805 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include "comm.h"
#include "util.h"
#include <signal.h>


int pipe_user_reading_from_server[2];//pipe_user_reading_from_server
int pipe_user_writing_to_server[2];//pipe_user_writing_to_server
//modified from lecture notes
void catchint(int sig) {
	write(pipe_user_writing_to_server[1],"signal",strlen("signal"));
	printf("\n");
	printf("Exit because pressed Ctrl+c\n");
	exit(-1);
}

/* -------------------------Main function for the client ----------------------*/
void main(int argc, char * argv[]) {
	//modified from lecture notes

	char buf[MAX_MSG]; 

	// You will need to get user name as a parameter, argv[1].

	if(connect_to_server("group66", argv[1], pipe_user_reading_from_server, pipe_user_writing_to_server) == -1) {
		exit(-1);
	}
	print_prompt(argv[1]);

	//non-block
	fcntl(0, F_SETFL, fcntl(0, F_GETFL)| O_NONBLOCK);
	fcntl(pipe_user_reading_from_server[0], F_SETFL, fcntl(pipe_user_reading_from_server[0], F_GETFL)| O_NONBLOCK);
	//close unused end
	close(pipe_user_reading_from_server[1]);
	close(pipe_user_writing_to_server[0]);
	//print error information for occupied user name and excessive users
	if(read(pipe_user_reading_from_server[0],buf,MAX_MSG)>0) {
		if(strcmp("User_name_already_used",buf)==0) {
			printf("User name already used\n");
        	exit(-1);
   		}

   		if(strcmp("No_slot",buf)==0) {
   			printf("No slot available\n");
   			exit(-1);
   		}
   	}

   		struct sigaction act; 
		act.sa_handler = catchint;
    	sigfillset (&act.sa_mask);
		sigaction (SIGINT, &act, NULL);
	


	while (1) {
		//sleep for some time
		usleep(1000);
		//read from server
		int nbytes2=read(pipe_user_reading_from_server[0],buf,MAX_MSG);
		//if read bytes smaller than 0 and the error code is not set to EAGAIN
		if(nbytes2<0 && errno!=EAGAIN) {
			perror("error reading data from pipe");
			exit(-1);
        }

        //print information to the terminal
        if (nbytes2>0) {
			 printf("%s\n", buf);
			 memset(buf,'\0', MAX_MSG);
			 print_prompt(argv[1]);
        }
        //broken pipe exit
        if(nbytes2==0) {
        	exit(-1);
        }

        //read from stdin
		int nbytes1=read(0,buf,MAX_MSG);
		if(nbytes2<0 && errno!=EAGAIN) {
			perror("error reading data from user terminal");
        	exit(-1);
        }
        //if read 0 from terminal,just continue the process
        if(nbytes1==0) {
        	continue;
        	print_prompt(argv[1]);
        }

        if (nbytes1>0) {
        	buf[strlen(buf)-1]='\0';
			if (write(pipe_user_writing_to_server[1],buf,strlen(buf))<0) {
				exit(-1);
			}
			if(strcmp("\\seg",buf)==0) {
				char *n=NULL;
				*n=1;
			}
		
			memset(buf,'\0', MAX_MSG);
			print_prompt(argv[1]);
        }


        //poll pipe

    }
   
		
 

	/* -------------- YOUR CODE STARTS HERE -----------------------------------*/

	
	// poll pipe retrieved and print it to sdiout

	// Poll stdin (input from the terminal) and send it to server (child process) via pipe

		
	/* -------------- YOUR CODE ENDS HERE -----------------------------------*/
}

/*--------------------------End of main for the client --------------------------*/


