#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "ftp_packet.h"

//#define	DEBUG
#undef	DEBUG
#define PKT_HEAD_LEN (sizeof(struct packet) - LENBUFFER)

static int packet_len(struct packet *hp) {
	return hp->datalen + PKT_HEAD_LEN;
}

void clear_packet(struct packet* p)
{
	short int t = p->conid;
	memset(p, 0, sizeof(struct packet));
	p->conid = t;
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

static void dump(const char *msg, const void *buf, int len) {
	int i;
	const unsigned char *packet = (const unsigned char *) buf;
	printf("%s", msg);
	for (i = 0; i < len; i++)
		printf(" %02x", packet[i]);
	putchar('\n');
}

void printpacket(const char *msg, const struct packet* p)
{
	printf("%s: conid(%d) type(%d) comid(%d) datalen(%d)\n", msg,
			p->conid, p->type, p->comid, p->datalen);
	dump("data:", p->buffer, p->datalen);
	fflush(stdout);
}


/**
 * 0 success, -1 error
 */
int send_packet(int sfd, struct packet* hp)
{
	int len;
	struct packet pkt;

	len = packet_len(hp);
	memcpy(&pkt, hp, len);
#ifdef	DEBUG
	printpacket("send", &pkt);
#endif
	if((send(sfd, htonp(&pkt), len, 0)) != len)
		return -1;

	return 0;
}

/**
 * read packet header, 0 success, -1 error
 */
static int recv_packet_header(int sfd, struct packet* pkt)
{
	int x;
	unsigned char *p = (unsigned char *)pkt;
	int rlen = PKT_HEAD_LEN;
	while(rlen > 0) {
		if((x = recv(sfd, p, rlen, 0)) <= 0)
			return -1;
		p += x;
		rlen -= x;
	}
	ntohp(pkt);
	return 0;
}

/**
 * 0 on success, or -1 for errors.
 */
static int recv_packet_data(int sfd, struct packet* pkt)
{
	int x;
	unsigned char *p = (unsigned char *)pkt->buffer;
	int rlen = pkt->datalen;
	while(rlen > 0) {
		if((x = recv(sfd, p, rlen, 0)) <= 0)
			return -1;
		p += x;
		rlen -= x;
	}
	return 0;
}

/**
 * 0 on success, or -1 for errors.
 */
int recv_packet(int sfd, struct packet* pkt)
{
	if (recv_packet_header(sfd, pkt) != 0)
		return -1;
	if (pkt->datalen > 0)
		if (recv_packet_data(sfd, pkt) != 0)
			return -1;

#ifdef	DEBUG
	printpacket("recv", pkt);
#endif

	return 0;
}

