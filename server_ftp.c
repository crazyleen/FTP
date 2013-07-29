#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include "ftp_packet.h"
#include "file_transfer_functions.h"
#include "server_ftp_functions.h"

#define Trace(...);    do { printf("%s:%u:TRACE: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf(" [%s]\n", __FUNCTION__); } while (0);
#define Debug(...);    do { printf("%s:%u:DEBUG: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf(" [%s]\n", __FUNCTION__); } while (0);
#define Info(...);		do { printf("%s:%u:INFO: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf(" [%s]\n", __FUNCTION__); } while (0);
#define Error(...);   	do { printf("%s:%u:ERROR: ", __FILE__, __LINE__); printf(__VA_ARGS__); perror(""); printf(" [%s]\n", __FUNCTION__); } while (0);

#define MAX_CLIENT 32

struct client_args {
	int sockfd;
	struct sockaddr_in addr;
};

static sem_t client_sem; //max connections
static int sockfd = 0;	//server socket
static int client_id[MAX_CLIENT];

void* serve_client(void*);

/**
 * malloc args for thread
 * return args, NULL for malloc error
 */
static void *thread_alloc_args(struct client_args *client) {
	void *args = malloc(sizeof(struct client_args));
	if(args == NULL) {
		Error("malloc");
		return NULL;
	}
	memcpy(args, client, sizeof(struct client_args));
	return args;
}

static void thread_free_args(void *args) {
	if(args != NULL)
		free(args);
}

static void thread_client_cleanup(void *args) {
	struct client_args *client = (struct client_args*)args;
	//client thread exit
	if(sem_post(&client_sem) < 0) {
		perror("sem_wait");
	}
	close(client->sockfd);
	thread_free_args(args);
}

static void *thread_client(void *args) {
	struct client_args *client = (struct client_args*)args;
	pthread_cleanup_push(thread_client_cleanup, args);

	struct client_info info;
	info.sfd = client->sockfd;
	info.cid = abs((int)pthread_self());

	pthread_exit(serve_client(&info));
	pthread_cleanup_pop(0);
}

static void at_exit_cleanup(void) {
	sem_destroy(&client_sem);
	close(sockfd);
}

/**
 * become a TCP server, bind to port, listen for maxconnect connections
 * return socket fd, -1 on error
 */
int socket_server(int port, int maxconnect) {
	int sockfd;
	struct sockaddr_in servaddr;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return -1;
	}

	int on = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);
	if ((bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))) < 0) {
		perror("listen");
		close(sockfd);
		return -1;
	}

	if (listen(sockfd, maxconnect) < 0) {
		perror("listen");
		close(sockfd);
		return -1;
	}

	return sockfd;
}

/**
 * write lock
 * @return: 0 on success,  <0 and (errno == EACCES || errno == EAGAIN) is locked file
 */
int lockfile(int fd) {
	struct flock fl;

	fl.l_type = F_WRLCK;
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;
	return (fcntl(fd, F_SETLK, &fl));
}

/**
 * open and set write lock
 * @return: return fd, -1 on error, -2 on locked file(failed to set lock)
 */
int open_file_lock(const char *file) {
	int fd;

	//open for lock
	fd = open(file, O_RDWR | O_CREAT, (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
	if (fd < 0) {
		printf("%s: can't open %s: %s", __FUNCTION__, file, strerror(errno));
		//open error
		return -1;
	}
	if (lockfile(fd) < 0) {
		if (errno == EACCES || errno == EAGAIN) {
			//locked file, can't lock file again
			close(fd);
			return -2;
		}
		//lock error
		printf("can't lock %s: %s", file, strerror(errno));
		return -1;
	}

	return fd;
}

/**
 * write_pid_lock - lock file and write pid
 * @return: return fd, -1 on locked file, -2 on error
 */
int write_pid_lock(const char *file) {
	int fd;
	char buf[18];

	fd = open_file_lock(file);
	if (fd < 0) {
		//fail, -1 for locked, -2 on error
		return fd;
	}

	//write pid to file
	if(ftruncate(fd, 0) < 0) {
		;
	}
	snprintf(buf, 18, "%ld\n", (long) getpid());
	if(write(fd, buf, strlen(buf) + 1) != strlen(buf) + 1) {
		printf("write %s to pid file error\n", buf);
	}

	return fd;
}

int main(int argc, char**argv) {
	const char *pidfile = "/var/run/ftp_multithread_server.pid";
	if(write_pid_lock(pidfile) < 0) {
		printf("%s: already running\n", argv[0]);
		return 0;
	}

	sockfd = socket_server(PORTSERVER, MAX_CLIENT);
	if (sockfd < 0) {
		Error("socket server");
		return -1;
	}

	if(sem_init(&client_sem, 0, MAX_CLIENT) < 0) {
		close(sockfd);
		Error("exit now");
		return -1;
	}

	memset(client_id, 0, sizeof(client_id));
	atexit(at_exit_cleanup);
	while (1) {
		size_t client_addr_len;
		struct client_args client;
		pthread_t thread;
		int ret;

		if(sem_wait(&client_sem) < 0) {
			perror("sem_wait");
			break;
		}

		bzero(&client, sizeof(client));
		client_addr_len = sizeof(client.addr);
		client.sockfd = accept(sockfd, (struct sockaddr *) &client.addr,
						&client_addr_len);
		if (client.sockfd < 0) {
			Error("accept");
			break;
		}
		//client count

		Info("client connect from %s", inet_ntoa(client.addr.sin_addr));
		void *thread_args = thread_alloc_args(&client);
		ret = pthread_create(&thread, NULL, thread_client, thread_args);
		if (ret != 0) {
			Error("pthread_create");
			if(thread_args != NULL)
				free(thread_args);
			if(sem_post(&client_sem) < 0) {
				Error("sem_wait");
			}
			continue;
		}
		pthread_detach(thread);
	}

	return 0;
}

void* serve_client(void* info)
{
	int sfd_client, connection_id, x;
	struct packet shp;
	char lpwd[LENBUFFER];
	struct client_info* ci = (struct client_info*) info;
	sfd_client = ci->sfd;
	connection_id = ci->cid;
	
	while(1)
	{
		if(recv_packet_ret(sfd_client, &shp) < 0)
		{
			fprintf(stderr, "client ID(%d) closed/terminated. closing connection.\n", connection_id);
			break;
		}

		if(shp.type == TERM)
			break;

		if(shp.conid == -1)
			shp.conid = connection_id;
		
		if(shp.type == REQU)
		{
			switch(shp.comid)
			{
				case PWD:
					if(!getcwd(lpwd, sizeof lpwd))
						er("getcwd()", 0);
					command_pwd(sfd_client, &shp, lpwd);
					break;
				case CD:
					if((x = chdir(shp.buffer)) == -1)
						fprintf(stderr, "Wrong path.\n");
					command_cd(sfd_client, &shp, x == -1 ? "fail" : "success");
					break;
				case MKDIR:
					command_mkdir(sfd_client, &shp);
					break;
				case LS:
					if(!getcwd(lpwd, sizeof lpwd))
						er("getcwd()", 0);
					command_ls(sfd_client, &shp, lpwd);
					break;
				case GET:
					command_get(sfd_client, &shp);
					break;
				case PUT:
					command_put(sfd_client, &shp);
					break;
				case RGET:
					if(!getcwd(lpwd, sizeof lpwd))
						er("getcwd()", 0);
					command_rget(sfd_client, &shp);
					send_EOT(sfd_client, &shp);
					if((x = chdir(lpwd)) == -1)
						fprintf(stderr, "Wrong path.\n");
					break;
				default:
					// print error
					break;
			}
		}
		else
		{
			//show error, send TERM and break
			fprintf(stderr, "packet incomprihensible. closing connection.\n");
			send_TERM(sfd_client, &shp);
			break;
		}
	}

	close(sfd_client);
	fflush(stdout);
	return NULL;
}

