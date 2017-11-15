#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "commands.h"
#include "built_in.h"
#include "signal.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <unistd.h>
#define SOCK_PATH "tpf_unix_sock.server"
#define DATA "Help from server"

#define UNIX_PATH_MAX 108
#define SERVER_PATH "tpf_unix_sock.server"
#define CLIENT_PATH "tpf_unix_sock.client"
void *client(struct single_command* commands);

static struct built_in_command built_in_commands[] = {
  { "cd", do_cd, validate_cd_argv },
  { "pwd", do_pwd, validate_pwd_argv },
  { "fg", do_fg, validate_fg_argv }
};

static int is_built_in_command(const char* command_name)
{
  static const int n_built_in_commands = sizeof(built_in_commands) / sizeof(built_in_commands[0]);

  for (int i = 0; i < n_built_in_commands; ++i) {
    if (strcmp(command_name, built_in_commands[i].command_name) == 0) {
      return i;
    }
  }

  return -1; // Not found
}

/*
 * Description: Currently this function only handles single built_in commands. You should modify this structure to launch process and offer pipeline functionality.
 */
int evaluate_command(int n_commands, struct single_command (*commands)[512])
{
  if (n_commands ==  1) {
	struct single_command* com = commands;
    assert(com->argc != 0);

    int built_in_pos = is_built_in_command(com->argv[0]);
    if (built_in_pos != -1) {
      if (built_in_commands[built_in_pos].command_validate(com->argc, com->argv)) {
        if (built_in_commands[built_in_pos].command_do(com->argc, com->argv) != 0) {
          fprintf(stderr, "%s: Error occurs\n", com->argv[0]);
        }
      } 
	  else {
        fprintf(stderr, "%s: Invalid arguments\n", com->argv[0]);
        return -1;
      }
    } 
	else if (strcmp(com->argv[0], "") == 0) {
      return 0;
    } 
	else if (strcmp(com->argv[0], "exit") == 0) {
      return 1;
    } 
	else {
		char path[5][30] = {"/usr/local/bin/", 
		                    "/usr/bin/", 
							"/bin/", 
							"/usr/sbin/",
							"/sbin/" };
        int status;
		int pid, pidnum;
        int bground = 0;

		if (!strcmp(com->argv[(com->argc)-1], "&")) {
			com->argv[(com->argc)-1] = NULL;
			(com->argc)--;
			bground = 1;
			}
		pid = fork();

		if(pid<0) {
			printf("Fork failed\n");
			return -1;
		}

		else if(pid == 0) {
		    if(bground == 1) {
				pidnum = getpid();
				printf("%d (Process ID)\n", pidnum);
				}
			if(execv(com->argv[0], com->argv) ==-1) {
				for(int i = 0; i < 5; i++) {
				  char tmp[100];
				  strcpy(tmp, path[i]);
				  strcpy(tmp+(strlen(path[i])), com->argv[0]);
			      execv(tmp, com->argv);
				  }
				
				fprintf(stderr, "%s: command not found\n", com->argv[0]);
				exit(0);
				return-1;
			}
		}

        else {
			if(bground == 0) wait(&status);
		}
    }
   }
  else if(n_commands > 1) { 
    int sSock, cSock, len, rc, status;
	int bytes_rec = 0;
	struct sockaddr_un sSockaddr;
	struct sockaddr_un cSockaddr;
	char buf[256];
	int backlog = 10;
	memset(&sSockaddr, 0, sizeof(struct sockaddr_un));
	memset(&cSockaddr, 0, sizeof(struct sockaddr_un));
	pthread_t cThread;
	int cThread_value;
	
	 
	cThread_value = 
	   pthread_create(&cThread, NULL, client, *commands);
	if(cThread_value < 0) {
		printf("Thread create error\n");
		exit(0);
	}

	sSock = socket(AF_UNIX, SOCK_STREAM, 0);
	if(sSock == -1) {
		printf("SOCKET ERROR\n");
		exit(1);
	}


	sSockaddr.sun_family = AF_UNIX;
	strcpy(sSockaddr.sun_path, SOCK_PATH);
	len = sizeof(sSockaddr);

	unlink(SOCK_PATH);
	rc = bind(sSock, (struct sockaddr *) &sSockaddr, len);
	if (rc == -1) {
		printf("BIND ERROR\n");
		close(sSock);
		exit(1);
	}

	rc = listen(sSock, backlog);
	if (rc == -1) {
		printf("LISTEN ERROR\n");
		close(sSock);
		exit(1);
	}

	cSock = accept(sSock, (struct sockaddr*) &cSockaddr, &len);
	if (cSock == -1) {
		printf("ACCEPT ERROR\n");
		close(sSock);
		close(cSock);
		exit(1);
	}

	pthread_join(cThread, NULL);

	if (fork() == 0) {
		close(0);
		dup2(cSock, 0);
		close(cSock);
		evaluate_command(1, *commands + 1);
		exit(0);
	}
	wait(&status);
//	pthread_join(cThread, NULL);
	close(sSock);
	close(cSock);
   
	}
  return 0;  
}

void *client(struct single_command* com1) {
   int cSock, rc, len, status;
   struct sockaddr_un sSockaddr;
   struct sockaddr_un cSockaddr;
   char buf[256];
   memset(&sSockaddr, 0, sizeof(struct sockaddr_un));
   memset(&cSockaddr, 0, sizeof(struct sockaddr_un));

   cSock = socket(AF_UNIX, SOCK_STREAM, 0);
   if(cSock == -1) {
   	printf("SOCKET ERROR\n");
	exit(1);
   }

   cSockaddr.sun_family = AF_UNIX;
   strcpy(cSockaddr.sun_path, CLIENT_PATH);
   len = sizeof(cSockaddr);

   unlink(CLIENT_PATH);
   rc = bind(cSock, (struct sockaddr *) &cSockaddr, len);
   if (rc == -1) {
   	printf("BIND ERROR\n");
	close(cSock);
	exit(1);
   }

   sSockaddr.sun_family = AF_UNIX;
   strcpy(sSockaddr.sun_path, SERVER_PATH);
   rc = connect(cSock, (struct sockaddr *) &sSockaddr, len);
   if(rc == -1) {
   	printf("CONNECT ERROR\n");
	close(cSock);
	exit(1);
   }
	if(fork() == 0 ) {
		close(1);
		dup2(cSock, 1);
		evaluate_command(1, com1);
		exit(0);
	}
	wait(&status);

   close(cSock);
   pthread_exit(NULL);
   return 0;
}
void free_commands(int n_commands, struct single_command (*commands)[512])
{
  for (int i = 0; i < n_commands; ++i) {
    struct single_command *com = (*commands) + i;
    int argc = com->argc;
    char** argv = com->argv;

    for (int j = 0; j < argc; ++j) {
      free(argv[j]);
    }

    free(argv);
  }

  memset((*commands), 0, sizeof(struct single_command) * n_commands);
}
