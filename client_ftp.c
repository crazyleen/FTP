#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ftp_packet.h"
#include "client_ftp_functions.h"

int main(int argc, char* argv[])
{
	//BEGIN: initialization
	struct sockaddr_in sin_server;
	int sfd_client;
	int x = 0;
	size_t size_sockaddr = sizeof(struct sockaddr);
	struct packet chp;

	char *server_ip = argv[1];
	if(argc != 2) {
		fprintf(stderr, "usage: %s <ip address>\n", argv[0]);
		exit(1);
	}

	if((sfd_client = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		er("socket()", x);
	
	memset((char*) &sin_server, 0, sizeof(struct sockaddr_in));
	sin_server.sin_family = AF_INET;
	sin_server.sin_addr.s_addr = inet_addr(server_ip);
	sin_server.sin_port = htons(PORTSERVER);
	
	if((x = connect(sfd_client, (struct sockaddr*) &sin_server, size_sockaddr)) < 0)
		er("connect()", x);
			
	printf(ID "FTP Client started up. Attempting communication with server @ %s:%d...\n\n", server_ip, PORTSERVER);
	//END: initialization

	clear_packet(&chp);
	chp.conid = -1;
	
	struct command* cmd;
	char lpwd[LENBUFFER];
	char userinput[LENUSERINPUT];
	while(1)
	{
		printf("\t> ");
		fgets(userinput, LENUSERINPUT, stdin);
					/* in order to give
					 * a filename with spaces, put ':'
					 * instead of ' '. If a command needs
					 * x paths, and y (y > x) paths are
					 * provided, y - x paths will be
					 * ignored.
					 * */
		cmd = userinputtocommand(userinput);
		if(!cmd)
			continue;
		//printcommand(cmd);
		switch(cmd->id)
		{
			case GET:
				if(cmd->npaths)
					command_get(sfd_client, &chp, *cmd->paths);
				else
					fprintf(stderr, "No path to file given.\n");
				break;
			case PUT:
				if(cmd->npaths)
					command_put(sfd_client, &chp, *cmd->paths);
				else
					fprintf(stderr, "No path to file given.\n");
				break;
			case MGET:
				if(cmd->npaths)
					command_mget(sfd_client, &chp, cmd->npaths, cmd->paths);
				else
					fprintf(stderr, "No path to file given.\n");
				break;
			case MPUT:
				if(cmd->npaths)
					command_mput(sfd_client, &chp, cmd->npaths, cmd->paths);
				else
					fprintf(stderr, "No path to file given.\n");
				break;
			case MGETWILD:
				command_mgetwild(sfd_client, &chp);
				break;
			case MPUTWILD:
				if(!getcwd(lpwd, sizeof lpwd))
					er("getcwd()", 0);
				command_mputwild(sfd_client, &chp, lpwd);
				break;
			case CD:
				if(cmd->npaths)
					command_cd(sfd_client, &chp, *cmd->paths);
				else
					fprintf(stderr, "No path given.\n");
				break;
			case LCD:
				if(cmd->npaths)
					command_lcd(*cmd->paths);
				else
					fprintf(stderr, "No path given.\n");
				break;
			case PWD:
				command_pwd(sfd_client, &chp);
				break;
			case LPWD:
				if(!getcwd(lpwd, sizeof lpwd))
					er("getcwd()", 0);
				printf("\t%s\n", lpwd);
				break;
			case DIR_:
			case LS:
				command_ls(sfd_client, &chp);
				break;
			case LDIR:
			case LLS:
				if(!getcwd(lpwd, sizeof lpwd))
					er("getcwd()", 0);
				command_lls(lpwd);
				break;
			case MKDIR:
				if(cmd->npaths)
					command_mkdir(sfd_client, &chp, *cmd->paths);
				else
					fprintf(stderr, "No path to directory given.\n");
				break;
			case LMKDIR:
				if(cmd->npaths)
					command_lmkdir(*cmd->paths);
				else
					fprintf(stderr, "No path to directory given.\n");
				break;
			case RGET:
				if(!getcwd(lpwd, sizeof lpwd))
					er("getcwd()", 0);
				command_rget(sfd_client, &chp);
				if((x = chdir(lpwd)) == -1)
					fprintf(stderr, "Wrong path.\n");
				break;
			case RPUT:
				if(!getcwd(lpwd, sizeof lpwd))
					er("getcwd()", 0);
				command_rput(sfd_client, &chp);
				if((x = chdir(lpwd)) == -1)
					fprintf(stderr, "Wrong path.\n");
				break;
			case EXIT:
				goto main_exit;
			default:
				// display error
				break;
		}
	}

main_exit:
	close(sfd_client);
	printf(ID "Done.\n");
	fflush(stdout);
	
	return 0;
}

