#include "common.h"
#include "interface.h"

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
    int n = 0, i = 1;
    if (argc <= 1) return n;
    for (; i < argc; i++) {
        if (argv[i][0] != '-') return -1;
        switch (argv[i][1]) {
            case 'n':
                *hostname = (char *)malloc(sizeof(char) * (MAX_HOST + 1));
                if (strlen(argv[i+1]) > MAX_HOST) // Prevent overflow
                    argv[i+1][MAX_HOST] = '\0';
                strncpy(*hostname, argv[i+1], MAX_HOST + 1);
                i++; // Ignore next word
                n++;
                break;
            case 'p':
                if (strlen(argv[i+1]) > MAX_PORT) // Prevent overflow
                    argv[i+1][MAX_PORT] = '\0';
                strncpy(port, argv[i+1], MAX_PORT + 1);
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

int is_start(const char * command, size_t command_length) {
    if (command_length == 5)
        return !memcmp(command, "start", command_length);
    if (command_length == 2)
        return !memcmp(command, "sg", command_length);
    return 0;
}

int is_play(const char * command, size_t command_length) {
    if (command_length == 4)
        return !memcmp(command, "play", command_length);
    if (command_length == 2)
        return !memcmp(command, "pl", command_length);
    return 0;
}

int is_guess(const char * command, size_t command_length) {
    if (command_length == 5)
        return !memcmp(command, "guess", command_length);
    if (command_length == 2)
        return !memcmp(command, "gw", command_length);
    return 0;
}

int is_quit(const char * command, size_t command_length) {
    if (command_length == 4) {
        return !memcmp(command, "quit", command_length);
    }
    return 0;
}

int is_exit(const char * command, size_t command_length){
    if (command_length == 4) {
        return !memcmp(command, "exit", command_length);
    }
    return 0;
}

int is_rev(const char * command, size_t command_length) {
    if (command_length == 3)
        return !memcmp(command, "rev", command_length);
    return 0;
}

int is_scoreboard(const char * command, size_t command_length) {
    if (command_length == 10)
        return !memcmp(command, "scoreboard", command_length);
    if (command_length == 2)
        return !memcmp(command, "sb", command_length);
    return 0;
}

int is_hint(const char * command, size_t command_length) {
    if (command_length == 4)
        return !memcmp(command, "hint", command_length);
    if (command_length == 1)
        return !memcmp(command, "h", command_length);
    return 0;
}

int is_state(const char * command, size_t command_length) {
    if (command_length == 5)
        return !memcmp(command, "state", command_length);
    if (command_length == 2)
        return !memcmp(command, "st", command_length);
    return 0;
}

int is_id(const char * plid, size_t plid_length){
    if(plid_length > 6){
        return 0;
    }
    for(int i = 0; i < plid_length; i++){
        if (plid[i]<'0' || plid[i]>'9'){
            return 0;
        }
    }
    return 1;
}

/*
Parses command from player to the according message protocol to server.
Returns:
    * -1, if not recognizable;
    * 0, if "exit";
    * 1, if "quit";
    * length, if it's sent through UDP (except for "exit" and "quit")
    * -length, if it's sent through TCP
*/
int parse_input(char * message, char * plid, int trial) {
    printf("inside\n");
    char buffer[WORD_MAX+8];
    char command[MAX_COMMAND+1];
    char info[WORD_MAX+1];
    char c[2];
    printf("fgets\n");
    fgets(buffer, WORD_MAX+8, stdin);
    printf("sscanf\n");
    sscanf(buffer, "%s %s", command, info);
    printf("start\n");
    if(is_start(command, strnlen(command, MAX_COMMAND))){
        printf("inside start\n");
        if(!is_id(info, strnlen(info, WORD_MAX))){
            return -1;
        }
        printf("memcpy\n");
        memcpy(plid, "000000", 6);
        memcpy((plid + 6 - strnlen(info, WORD_MAX)), info, 6);
        //memcpy(plid, info, strnlen(info, WORD_MAX));
        printf("snprintf\n");
        snprintf(message, 12, "SNG %s\n", plid);
        printf(message);
        printf("return\n");
        return strnlen(message, 11);
    }
    else if(is_play(command, strnlen(command, MAX_COMMAND))){
        if(plid == NULL){
            return -1;
        }
        if(strnlen(info, WORD_MAX) != 1 || info[0] < 'A' || info[0] > 'z' || (info[0] > 'Z' && info[0] < 'a')){
            return -1;
        }
        printf("is play\n");
        memcpy(c, info, strnlen(info, WORD_MAX));
        snprintf(message, 16, "PLG %s %s %d\n", plid, c, trial);
        return strnlen(message, 16);
    }
    else if(is_guess(command, strnlen(command, MAX_COMMAND))){
        if(plid == NULL){
            return -1;
        }
        printf("is guess\n");
        snprintf(message, 15+WORD_MAX, "PWG %s %s %d\n", plid, info, trial);
        return strnlen(message, 15+WORD_MAX);
    }
    else if(is_rev(command, strnlen(command, MAX_COMMAND))){
        if(plid == NULL){
            return -1;
        }
        printf("is rev\n");
        snprintf(message, 11, "REV %s\n", plid);
        return strnlen(message, 11);
    }
    else if(is_quit(command, strnlen(command, MAX_COMMAND))){
        if(plid == NULL){
            return -1;
        }
        printf("is quit\n");
        snprintf(message, 11, "QUT %s\n", plid);
        return 1;
    }
    else if(is_exit(command, strnlen(command, MAX_COMMAND))){
        if(plid == NULL){
            return -1;
        }
        printf("is exit\n");
        snprintf(message, 11, "QUT %s\n", plid);
        return 0;
    }
    else if(is_scoreboard(command, strnlen(command, MAX_COMMAND))){
        if(plid == NULL){
            return -1;
        }
        printf("is scoreboard\n");
        memcpy(message, "GSB\n", sizeof("GSB\n"));
        return strnlen(message, 4);
    }
    else if(is_hint(command, strnlen(command, MAX_COMMAND))){
        if(plid == NULL){
            return -1;
        }
        printf("is hint\n");
        snprintf(message, 11, "GHL %s\n", plid);
        return strnlen(message, 11);
    }
    else if(is_state(command, strnlen(command, MAX_COMMAND))){
        if(plid == NULL){
            return -1;
        }
        printf("is state\n");
        snprintf(message, 11, "STA %s\n", plid);
        return strnlen(message, 11);
    }
    printf("Error in parsing\n");
    // ? Obter palavra de input -> Redirecionar -> Obter mais se for preciso ?
    // Nova ideia
    // ? Estrutura CMD -> redirecionar para comando certo -> Preencher campos CMD -> traduzir CMD p/ string
    return -1;
}

ssize_t complete_write(int fd, char * buffer, ssize_t n) {
    ssize_t nleft = n;
    ssize_t nwritten = 0;
    char *ptr = buffer;

    while (nleft > 0) {
        nwritten = write(fd, ptr, nleft);
        if (nwritten <= 0) return -1;
        nleft -= nwritten;
        ptr += nwritten;
    }
    return n;
}

ssize_t complete_read(int fd, char * buffer, ssize_t n) {
    ssize_t nleft = n;
    ssize_t nread = 0;
    char *ptr = buffer;

    while (nleft > 0) {
        nread = read(fd, ptr, nleft);
        if (nread == -1) return -1;
        else if (nread == 0) return n - nleft;
        nleft -= nread;
        ptr += nread;
    }
    return n;
}