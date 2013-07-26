#include <commons.h>
#include <file_transfer_functions.h>

#define IPSERVER	"127.0.0.1"
#define	ID		"CLIENT=> "

#define LENUSERINPUT	1024

struct command
{
	short int id;
	int npaths;
	char** paths;
};

struct command* userinputtocommand(char [LENUSERINPUT]);

void printcommand(struct command*);

void command_pwd(int, struct packet*);
void command_lcd(char*);
void command_cd(int, struct packet*, char*);
void command_lls(char*);
void command_ls(int, struct packet*);
void command_get(int, struct packet*, char*);
void command_put(int, struct packet*, char*);
void command_mget(int, struct packet*, int, char**);
void command_mput(int, struct packet*, int, char**);
void command_mgetwild(int, struct packet*);
void command_mputwild(int, struct packet*, char*);
void command_lmkdir(char*);
void command_mkdir(int, struct packet*, char*);
void command_rget(int, struct packet*);
void command_rput(int, struct packet*);

