#include <commons.h>

static size_t size_packet = sizeof(struct packet);


void set0(struct packet* p)
{
	memset(p, 0, sizeof(struct packet));
}

struct packet* ntohp(struct packet* np)
{
	np->conid = ntohs(np->conid);
	np->type = ntohs(np->type);
	np->comid = ntohs(np->comid);
	np->datalen = ntohs(np->datalen);
	return np;
}

struct packet* htonp(struct packet* hp)
{
	hp->conid = ntohs(hp->conid);
	hp->type = ntohs(hp->type);
	hp->comid = ntohs(hp->comid);
	hp->datalen = ntohs(hp->datalen);
	return hp;
}

void printpacket(struct packet* p, int ptype)
{
	if(!DEBUG)
		return;
	
	if(ptype)
		printf("\t\tHOST PACKET\n");
	else
		printf("\t\tNETWORK PACKET\n");
	
	printf("\t\tconid = %d\n", p->conid);
	printf("\t\ttype = %d\n", p->type);
	printf("\t\tcomid = %d\n", p->comid);
	printf("\t\tdatalen = %d\n", p->datalen);
	printf("\t\tbuffer = %s\n", p->buffer);
	
	fflush(stdout);
}


/**
 * if error occur, exit(-1)
 */
void send_packet(int sfd, struct packet* hp)
{
	int x;
	struct packet pkt;
	memcpy(&pkt, hp, size_packet);
	if((x = send(sfd, htonp(&pkt), size_packet, 0)) != size_packet)
		er("send()", x);
}

/**
 * Returns the number read or exit(-1) for errors.
 */
int recv_packet(int sfd, struct packet* pkt)
{
	int x;
	if((x = recv(sfd, pkt, size_packet, 0)) <= 0)
		er("recv()", x);
	ntohp(pkt);
	return x;
}
