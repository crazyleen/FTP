#include <server_ftp.h>

static size_t size_packet = sizeof(struct packet);

struct client_info* client_info_alloc(int s, int c)
{
	struct client_info* ci = (struct client_info*) malloc(sizeof(struct client_info));
	ci->sfd = s;
	ci->cid = c;
	return ci;
}

void command_pwd(int sfd_client, struct packet* shp, char* lpwd)
{
	int x;
	shp->type = DATA;
	strcpy(shp->buffer, lpwd);
	send_packet(sfd_client, shp);
}

void command_cd(int sfd_client, struct packet* shp, char* message)
{
	int x;
	shp->type = INFO;
	strcpy(shp->buffer, message);
	send_packet(sfd_client, shp);
}

void command_ls(int sfd_client, struct packet* shp, char* lpwd)
{
	int x;
	shp->type = DATA;
	DIR* d = opendir(lpwd);
	if(!d)
		er("opendir()", (int) d);
	struct dirent* e;
	while(e = readdir(d))
	{
		sprintf(shp->buffer, "%s\t%s", e->d_type == 4 ? "DIR:" : e->d_type == 8 ? "FILE:" : "UNDEF:", e->d_name);
		send_packet(sfd_client, shp);
	}
	send_EOT(sfd_client, shp);
}

void command_get(int sfd_client, struct packet* shp)
{
	int x;
	FILE* f = fopen(shp->buffer, "rb");	// Yo!
	shp->type = INFO;
	shp->comid = GET;
	strcpy(shp->buffer, f ? "File found; processing" : "Error opening file.");
	//printpacket(shp, HP);
	send_packet(sfd_client, shp);
	if(f)
	{
		shp->type = DATA;
		send_file(sfd_client, shp, f);
		fclose(f);
	}
	send_EOT(sfd_client, shp);
}

void command_put(int sfd_client, struct packet* shp)
{
	int x;
	FILE* f = fopen(shp->buffer, "wb");
	shp->type = INFO;
	shp->comid = PUT;
	strcpy(shp->buffer, f ? "Everything in order; processing" : "Error opening file for writing on server side.");
	//printpacket(shp, HP);
	send_packet(sfd_client, shp);
	if(f)
	{
		receive_file(sfd_client, shp, f);
		fclose(f);
	}
}

void command_mkdir(int sfd_client, struct packet* shp)
{
	char message[LENBUFFER];
	DIR* d = opendir(shp->buffer);
	if(d)
	{	
		strcpy(message, "already exists");
		closedir(d);
	}
	else if(mkdir(shp->buffer, 0777) == -1)
	{
		fprintf(stderr, "Wrong path.\n");
		strcpy(message, "fail");
	}
	else
		strcpy(message, "success");
	int x;
	shp->type = INFO;
	strcpy(shp->buffer, message);
	send_packet(sfd_client, shp);
}

void command_rget(int sfd_client, struct packet* shp)
{
	static char lpwd[LENBUFFER];
	if(!getcwd(lpwd, sizeof lpwd))
		er("getcwd()", 0);
	int x;
	DIR* d = opendir(lpwd);
	if(!d)
		er("opendir()", (int) d);
	struct dirent* e;
	while(e = readdir(d))
		if(e->d_type == 4 && strcmp(e->d_name, ".") && strcmp(e->d_name, ".."))
		{
			shp->type = REQU;
			shp->comid = LMKDIR;
			strcpy(shp->buffer, e->d_name);
			send_packet(sfd_client, shp);
			
			shp->type = REQU;
			shp->comid = LCD;
			strcpy(shp->buffer, e->d_name);
			send_packet(sfd_client, shp);
			if((x = chdir(e->d_name)) == -1)
				er("chdir()", x);

			command_rget(sfd_client, shp);
			
			shp->type = REQU;
			shp->comid = LCD;
			strcpy(shp->buffer, "..");
			send_packet(sfd_client, shp);
			if((x = chdir("..")) == -1)
				er("chdir()", x);
		}
		else if(e->d_type == 8)
		{
			shp->type = REQU;
			shp->comid = GET;
			strcpy(shp->buffer, e->d_name);
			send_packet(sfd_client, shp);
			recv_packet(sfd_client, shp);
			if(shp->type == REQU && shp->comid == GET)
				command_get(sfd_client, shp);
		}
	closedir(d);
}

