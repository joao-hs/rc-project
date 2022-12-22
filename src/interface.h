#include "common.h"

#define QUT_MESSAGE_LEN CMD_ID_LEN + 1 + PLID_LEN + 1

typedef struct cmd {
    char id[CMD_ID_LEN];
    char plid[PLID_LEN];
    char word[WORD_MAX];
    char letter;
    int trial_no;
} CMD;

typedef struct feedback {
    char id[CMD_ID_LEN];
    char status[STATUS_LEN];
    char word[WORD_MAX];
    int trial_no;
    int hit_no;
    int *pos;
    F_INFO *f;
} Feedback;

int is_valid_fname(char *fname);
int parse_input(char *message);
int parse_tcp_header(int fd, F_INFO *f);
int process_udp_response(char *response, int udp_code);
int process_udp_message(char *response, char *message);
int process_tcp_message(char *response, char *message, FILE **fp);