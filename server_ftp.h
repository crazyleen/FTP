#include <commons.h>
#include <file_transfer_functions.h>

#include <time.h>
/*
	for:
		ctime()
*/

#include <sys/stat.h>
/*
	for:
		stat()
*/

#define	ID	"SERVER=> "

struct client_info
{
	int sfd;
	int cid;
};

struct client_info* client_info_alloc(int s, int c);

void command_pwd(int sfd_client, struct packet* shp, char* lpwd);
void command_cd(int sfd_client, struct packet* shp, char* message);
void command_ls(int sfd_client, struct packet* shp, char* lpwd);
void command_get(int sfd_client, struct packet* shp);
void command_put(int sfd_client, struct packet* shp);
void command_mkdir(int sfd_client, struct packet* shp);
void command_rget(int sfd_client, struct packet* shp);


