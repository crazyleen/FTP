#ifndef COMMONS_H
#define COMMONS_H

#include <unistd.h>
#include <stdlib.h>

#define	DEBUG		1

#define PORTSERVER	8487
#define CONTROLPORT	PORTSERVER
#define DATAPORT	(PORTSERVER + 1)

enum TYPE
	{
		REQU,
		DONE,
		INFO,
		TERM,
		DATA,
		EOT
	};

#define	NP		0
#define	HP		1

#define	er(e, x)					\
	do						\
	{						\
		perror("ERROR IN: " #e "\n");		\
		fprintf(stderr, "%d\n", x);		\
		exit(-1);				\
	}						\
	while(0)

#define LENBUFFER	496		// so as to make the whole packet well-rounded ( = 512 bytes)
struct packet
{
	int32_t conid;
	int32_t type;
	int32_t comid;
	int32_t datalen;
	char buffer[LENBUFFER];
}__attribute__((__packed__));

void clear_packet(struct packet*);

struct packet* ntohp(struct packet*);
struct packet* htonp(struct packet*);

void printpacket(struct packet*, int);

/**
 * if error occur, exit(-1)
 */
void send_packet(int sfd, struct packet* data);

/**
 * exit(-1) for errors.
 */
void recv_packet(int sfd, struct packet* pkt);

#define NCOMMANDS 19
enum COMMAND
	{
		GET,
		PUT,
		MGET,
		MPUT,
		CD,
		LCD,
		MGETWILD,
		MPUTWILD,
		DIR_,		// _ to avoid conflict with directory pointer DIR
		LDIR,
		LS,
		LLS,
		MKDIR,
		LMKDIR,
		RGET,
		RPUT,
		PWD,
		LPWD,
		EXIT
	};			/* any change made here should also be
				replicated accordingly in the commandlist
				2D array in client_ftp_fucntions.c */

#endif

