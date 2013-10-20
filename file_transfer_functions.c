#include <stdio.h>
#include <string.h>
#include "ftp_packet.h"
#include "file_transfer_functions.h"

void send_EOT(int sfd, struct packet* hp)
{
	hp->type = EOT;
	hp->datalen = 0;
	send_packet(sfd, hp);
}

void send_TERM(int sfd, struct packet* hp)
{
	hp->type = TERM;
	hp->datalen = 0;
	send_packet(sfd, hp);
}

void send_file(int sfd, struct packet* hp, FILE* f)
{
	int i = 0, j = 0;
	while(!feof(f))
	{
		memset(hp->buffer, '\0', sizeof(char) * LENBUFFER);
		hp->datalen = fread(hp->buffer, 1, LENBUFFER - 1, f);
		i += hp->datalen;
		send_packet(sfd, hp);
		j++;
	}
	fprintf(stderr, "\t%d byte(s) read.\n", i);
	fprintf(stderr, "\t%d data packet(s) sent.\n", j);
	fflush(stderr);
}

void receive_file(int sfd, struct packet* hp, FILE* f)
{
	int i = 0, j = 0;
	recv_packet(sfd, hp);
	j++; //receive one data packet before loop
	//printpacket(hp, HP);
	while(hp->type == DATA)
	{
		i += fwrite(hp->buffer, 1, hp->datalen, f);
		recv_packet(sfd, hp);
		j++;
		//printpacket(hp, HP);
	}
	fprintf(stderr, "\t%d data packet(s) received.\n", --j);	// j decremented because the last packet is EOT.
	fprintf(stderr, "\t%d byte(s) written.\n", i);
	if(hp->type == EOT)
		return;
	else
	{
		fprintf(stderr, "Error occured while downloading remote file.\n");
		exit(2);
	}
	fflush(stderr);
}

