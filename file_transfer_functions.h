#ifndef FILE_TRANSFER_FUNCTIONS_H
#define FILE_TRANSFER_FUNCTIONS_H

#define extract_filename(filepath) ((strrchr(filepath, '/') != NULL) ? (strrchr(filepath, '/') + 1) : filepath)

void send_EOT(int sfd, struct packet* hp);
void send_TERM(int sfd, struct packet* hp);

void send_file(int sfd, struct packet* hp, FILE* f);
void receive_file(int sfd, struct packet* hp, FILE* f);

#endif
