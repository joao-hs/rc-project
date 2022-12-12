#include "common.h"
#include "interface.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define SPACE ' '
#define ENDLN '\n'
#define START_MSG "SNG"
/*
./player -n <name_server> -p <port>
*/

/* UDP messages
Commands:                               Send:                               Receive
start <PLID(ex. 095538)>\n              SNG <PLID>\n                        RSG <status> [<n_letters> <max_errors>]\n
sg <PLID(ex. 095538)>\n                 SNG <PLID>\n                        RSG <status> [<n_letters> <max_errors>]\n

play <char>\n                           PLG <PLID> <char> <trial_no>\n      RLG <status> [<trial> <n> <pos>*]\n
pl <char>\n                             PLG <PLID> <char> <trial_no>\n      RLG <status> [<trial> <n> <pos>*]\n

guess <word>\n                          PWG <PLID> <word> <trial_no>\n      RWG <status> <trials>\n
gw <word>\n                             PWG <PLID> <word> <trial_no>\n      RWG <status> <trials>\n

quit\n                                  QUT <PLID>\n                        RQT <status>\n
exit\n                                  QUT <PLID>\n                        RQT <status>\n

rev\n                                   REV <PLID>\n                        RRV <word>|<status>\n
*/

/* TCP messages
Commands:                               Send:                               Receive
scoreboard\n                            GSB\n                               RSB <status> [<Fname> <Fsize> <Fdata>]\n
sb\n                                    GSB\n                               RSB <status> [<Fname> <Fsize> <Fdata>]\n

hint\n                                  GHL <PLID>\n                        RHL <status> [<Fname> <Fsize> <Fdata>]\n
h\n                                     GHL <PLID>\n                        RHL <status> [<Fname> <Fsize> <Fdata>]\n

state\n                                 STA <PLID>\n                        RST <status> [<Fname> <Fsize> <Fdata>]\n
st\n                                    STA <PLID>\n                        RST <status> [<Fname> <Fsize> <Fdata>]\n
*/

/*
Parse command line arguments and set the variables accordingly
*/
int parse_cli(int argc, char * argv[], char ** hostname, char * port, int * verbose) {
    size_t len;
    int n = 0, i = 1;
    if (argc <= 1) return n;
    for (; i < argc; i++) {
        if (argv[i][0] != '-') return -1;
        switch (argv[i][1]) {
            case 'n':
                *hostname = (char *)malloc(sizeof(char) * (MAXHOST + 1));
                if ((len = strlen(argv[i+1])) > MAXHOST) // Prevent overflow
                    argv[i+1][MAXHOST] = '\0';
                strncpy(*hostname, argv[i+1], MAXHOST + 1);
                i++; // Ignore next word
                n++;
                break;
            case 'p':
                if ((len = strlen(argv[i+1])) > MAXPORT) // Prevent overflow
                    argv[i+1][MAXPORT] = '\0';
                strncpy(port, argv[i+1], MAXPORT + 1);
                i++; // Ignore next word
                n++;
                break;
            case 'v':
                *verbose = TRUE;
                n++;
                break;
            default:
                return -1;
        }   
    }
    return n;
}

/*
Translates input from player to readable message to server.
Returns -1 if not recognizable.
*/
int parse_input(char * message) {
    // ? Obter palavra de input -> Redirecionar -> Obter mais se for preciso ?
    // Nova ideia
    // ? Estrutura CMD -> redirecionar para comando certo -> Preencher campos CMD -> traduzir CMD p/ string
    return 0;
}

int cmd_start(char * dest, char * plid) {
    char * ptr = dest;
    char padded_plid[] = "000000";
    memcpy(ptr, START_MSG, sizeof(START_MSG) - 1);
    ptr += sizeof(START_MSG) - 1;
    memcpy(ptr, SPACE, sizeof(SPACE) - 1);
    ptr += sizeof(SPACE);

    if (snprintf(padded_plid, MAX_PLID, "%06d", plid) == -1) return -1;
    memcpy(ptr, padded_plid, sizeof(MAX_PLID));
    ptr += sizeof(MAX_PLID);
    memcpy(ptr, ENDLN, sizeof(ENDLN));
    return 0;
}

void init_cmd(CMD * new_cmd) {
    if (new_cmd == NULL) return;
    new_cmd->id[0] = '\0';
    new_cmd->plid[0] = '\0';
    new_cmd->word[0] = '\0';
    new_cmd->letter = '\0';
    new_cmd->trial_no = -1;
    return;
}

int cmd_start(CMD * dest, char * plid) {
    size_t len;
    memcpy(dest->id, START_MSG, sizeof(START_MSG));
    if ((len = strlen(plid)) == PLID_LEN)
        memcpy(dest->plid, plid, PLID_LEN);
    else if (len < PLID_LEN){
        memcpy(dest->plid, "000000", PLID_LEN);
        memcpy(dest->plid + PLID_LEN - len, plid, len);
    } else return -1;
    return 0;
}