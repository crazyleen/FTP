#ifndef COMMONS_H
#define COMMONS_H

#include <unistd.h>
#include <stdlib.h>


#define PORTSERVER	8487
#define CONTROLPORT	PORTSERVER
#define DATAPORT	(PORTSERVER + 1)

#define er(x, y) do { exit(y); } while(0)

enum PKT_TYPE
	{
		REQU,
		DONE,
		INFO,
		TERM,
		DATA,
		EOT
	};

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

void printpacket(const char *msg, const struct packet* p);

/**
 * 0 on success, -1 on error
 */
int send_packet(int sfd, struct packet* data);

/**
 * 0 on success, or -1 for errors.
 */
int recv_packet(int sfd, struct packet* pkt);

#define NCOMMANDS 19
enum PKT_COMMAND
	{
		GET,
		PUT,
		MGET,
		MPUT,
		CD,
		LCD,
		MGETWILD,
		MPUTWILD,
		DIR_,
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
	};

#endif

