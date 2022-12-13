#include "common.h"

#define QUT_MESSAGE_LEN CMD_ID_LEN + 1 + PLID_LEN + 1

typedef struct cmd {
    char id[CMD_ID_LEN];
    char plid[PLID_LEN];
    char word[WORD_MAX];
    char letter;
    int trial_no;
} CMD;

typedef struct file_info {
    char * f_name;
    int f_size;
    int * f_data;
} File;

typedef struct feedback {
    char id[CMD_ID_LEN];
    char status[STATUS_LEN];
    char word[WORD_MAX];
    int trial_no;
    int hit_no;
    int * pos;
    File * file;
} Feedback;

int parse_cli(int argc, char * argv[], char ** hostname, char * port, int * verbose);
int parse_input(char * message, int *pid);