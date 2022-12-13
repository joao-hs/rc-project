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
Parses command from player to the according message protocol to server.
Returns:
    * -1, if not recognizable;
    * 0, if "exit";
    * 1, if "quit";
    * any number < 0, if it's sent through TCP
    * any number >= 0, if it's sent through UDP
*/
int parse_input(char * message, int * player_id) {
    // ? Obter palavra de input -> Redirecionar -> Obter mais se for preciso ?
    // Nova ideia
    // ? Estrutura CMD -> redirecionar para comando certo -> Preencher campos CMD -> traduzir CMD p/ string
    return 0;
}

int is_start(const char * command, size_t command_length) {
    if (command_length == 5)
        return memcmp(command, "start", command_length);
    if (command_length == 2)
        return memcmp(command, "sg", command_length);
    return -1;
}

int is_play(const char * command, size_t command_length) {
    if (command_length == 4)
        return memcmp(command, "play", command_length);
    if (command_length == 2)
        return memcmp(command, "pl", command_length);
    return -1;
}

int is_guess(const char * command, size_t command_length) {
    if (command_length == 5)
        return memcmp(command, "guess", command_length);
    if (command_length == 2)
        return memcmp(command, "gw", command_length);
    return -1;
}

/*
If command is "quit", returns 0. If command is "exit" returns 1.
Else returns -1.
*/
int is_quit(const char * command, size_t command_length) {
    if (command_length == 4) {
        if (memcmp(command, "quit", command_length) == 0)
            return 0;
        if (memcmp(command, "exit", command_length) == 0)
            return 1;
    }
    return -1;
}

int is_rev(const char * command, size_t command_length) {
    if (command_length == 3)
        return memcmp(command, "rev", command_length);
    return -1;
}

int is_scoreboard(const char * command, size_t command_length) {
    if (command_length == 10)
        return memcmp(command, "scoreboard", command_length);
    if (command_length == 2)
        return memcmp(command, "sb", command_length);
    return -1;
}

int is_hint(const char * command, size_t command_length) {
    if (command_length == 4)
        return memcmp(command, "hint", command_length);
    if (command_length == 1)
        return memcmp(command, "h", command_length);
    return -1;
}

int is_state(const char * command, size_t command_length) {
    if (command_length == 5)
        return memcmp(command, "state", command_length);
    if (command_length == 2)
        return memcmp(command, "st", command_length);
    return -1;
}
