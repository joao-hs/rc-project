#include "interface.h"
#include "games.h"
#include "session.h"
#include "socket.h"

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

play <char>\n                           PLG <PLID> <char> <trial_no>\n      RLG <status> <trial> [<n> <pos>*]\n
pl <char>\n                             PLG <PLID> <char> <trial_no>\n      RLG <status> <trial> [<n> <pos>*]\n

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
int parse_cli(int argc, char *argv[], char **hostname, char *port, int *verbose) {
    int n = 0, i = 1;
    if (argc <= 1)
        return n;
    for (; i < argc; i++) {
        if (argv[i][0] != '-')
            return -1;
        switch (argv[i][1]) {
        case 'n':
            *hostname = (char *)malloc(sizeof(char) * (MAX_HOST + 1));
            if (strlen(argv[i + 1]) > MAX_HOST) // Prevent overflow
                argv[i + 1][MAX_HOST] = '\0';
            strncpy(*hostname, argv[i + 1], MAX_HOST + 1);
            i++; // Ignore next word
            n++;
            break;
        case 'p':
            if (strlen(argv[i + 1]) > MAX_PORT) // Prevent overflow
                argv[i + 1][MAX_PORT] = '\0';
            strncpy(port, argv[i + 1], MAX_PORT + 1);
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

/* COMMAND VERIFIERS */

int is_start(const char *command, size_t command_length) {
    if (command_length == 5)
        return !memcmp(command, "start", command_length);
    if (command_length == 2)
        return !memcmp(command, "sg", command_length);
    return 0;
}

int is_play(const char *command, size_t command_length) {
    if (command_length == 4)
        return !memcmp(command, "play", command_length);
    if (command_length == 2)
        return !memcmp(command, "pl", command_length);
    return 0;
}

int is_guess(const char *command, size_t command_length) {
    if (command_length == 5)
        return !memcmp(command, "guess", command_length);
    if (command_length == 2)
        return !memcmp(command, "gw", command_length);
    return 0;
}

int is_quit(const char *command, size_t command_length) {
    if (command_length == 4) {
        return !memcmp(command, "quit", command_length);
    }
    return 0;
}

int is_exit(const char *command, size_t command_length) {
    if (command_length == 4) {
        return !memcmp(command, "exit", command_length);
    }
    return 0;
}

int is_rev(const char *command, size_t command_length) {
    if (command_length == 3)
        return !memcmp(command, "rev", command_length);
    return 0;
}

int is_scoreboard(const char *command, size_t command_length) {
    if (command_length == 10)
        return !memcmp(command, "scoreboard", command_length);
    if (command_length == 2)
        return !memcmp(command, "sb", command_length);
    return 0;
}

int is_hint(const char *command, size_t command_length) {
    if (command_length == 4)
        return !memcmp(command, "hint", command_length);
    if (command_length == 1)
        return !memcmp(command, "h", command_length);
    return 0;
}

int is_state(const char *command, size_t command_length) {
    if (command_length == 5)
        return !memcmp(command, "state", command_length);
    if (command_length == 2)
        return !memcmp(command, "st", command_length);
    return 0;
}

int is_id(const char *plid, size_t plid_length) {
    if (plid_length > 6) {
        return 0;
    }
    for (int i = 0; i < plid_length; i++) {
        if (plid[i] < '0' || plid[i] > '9') {
            return 0;
        }
    }
    return 1;
}

/* RESPONSE COMMAND ID VERIFIERS */

int is_r_start(const char *cmd_id) {
    return !memcmp(cmd_id, "RSG\0", CMD_ID_LEN + 1);
}

int is_r_play(const char *cmd_id) {
    return !memcmp(cmd_id, "RLG\0", CMD_ID_LEN + 1);
}

int is_r_guess(const char *cmd_id) {
    return !memcmp(cmd_id, "RWG\0", CMD_ID_LEN + 1);
}

int is_r_quit(const char *cmd_id) {
    return !memcmp(cmd_id, "RQT\0", CMD_ID_LEN + 1);
}

int is_r_rev(const char *cmd_id) {
    return !memcmp(cmd_id, "RRV\0", CMD_ID_LEN + 1);
}

int is_r_scoreboard(const char *cmd_id) {
    return !memcmp(cmd_id, "RSB\0", CMD_ID_LEN + 1);
}

int is_r_hint(const char *cmd_id) {
    return !memcmp(cmd_id, "RHL\0", CMD_ID_LEN + 1);
}

int is_r_state(const char *cmd_id) {
    return !memcmp(cmd_id, "RST\0", CMD_ID_LEN + 1);
}

/* MESSAGE COMMAND VERIFIERS */

int is_m_start(const char *cmd_id) {
    return !memcmp(cmd_id, "SNG\0", CMD_ID_LEN + 1);
}

int is_m_play(const char *cmd_id) {
    return !memcmp(cmd_id, "PLG\0", CMD_ID_LEN + 1);
}

int is_m_guess(const char *cmd_id) {
    return !memcmp(cmd_id, "PWG\0", CMD_ID_LEN + 1);
}

int is_m_quit(const char *cmd_id) {
    return !memcmp(cmd_id, "QUT\0", CMD_ID_LEN + 1);
}

int is_m_rev(const char *cmd_id) {
    return !memcmp(cmd_id, "REV\0", CMD_ID_LEN + 1);
}

int is_m_scoreboard(const char *cmd_id) {
    return !memcmp(cmd_id, "GSB\0", CMD_ID_LEN + 1);
}

int is_m_hint(const char *cmd_id) {
    return !memcmp(cmd_id, "GHL\0", CMD_ID_LEN + 1);
}

int is_m_state(const char *cmd_id) {
    return !memcmp(cmd_id, "STA\0", CMD_ID_LEN + 1);
}

/* STATUS VERIFIERS */

int is_st_ok(const char *status) {
    return !memcmp(status, "OK\0", 3);
}

int is_st_win(const char *status) {
    return !memcmp(status, "WIN\0", 4);
}

int is_st_act(const char *status) {
    return !memcmp(status, "ACT\0", 4);
}

int is_st_fin(const char *status) {
    return !memcmp(status, "FIN\0", 4);
}

int is_st_dup(const char *status) {
    return !memcmp(status, "DUP\0", 4);
}

int is_st_nok(const char *status) {
    return !memcmp(status, "NOK\0", 4);
}

int is_st_ovr(const char *status) {
    return !memcmp(status, "OVR\0", 4);
}

int is_st_inv(const char *status) {
    return !memcmp(status, "INV\0", 4);
}

int is_st_err(const char *status) {
    return !memcmp(status, "ERR\0", 4);
}

int is_st_empty(const char *status) {
    return !memcmp(status, "EMPTY\0", 4);
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
int parse_input(char *message) {
    char buffer[WORD_MAX + 8];
    char command[MAX_COMMAND + 1];
    char info[WORD_MAX + 1];
    char c;
    fgets(buffer, WORD_MAX + 8, stdin);
    sscanf(buffer, "%s %s", command, info);
    if (is_start(command, strnlen(command, MAX_COMMAND))) {
        if (!is_id(info, strnlen(info, WORD_MAX))) {
            return -1;
        }
        init_game(info);
        snprintf(message, CMD_ID_LEN + 1 + PLID_LEN + 2, "SNG %s\n", get_plid());
        return strnlen(message, CMD_ID_LEN + 1 + PLID_LEN + 1);
    } else if (is_play(command, strnlen(command, MAX_COMMAND))) {
        if (get_plid() == NULL) {
            return -1;
        }
        if (strnlen(info, WORD_MAX) != 1 || info[0] < 'A' || info[0] > 'z' || (info[0] > 'Z' && info[0] < 'a')) {
            return -1;
        }
        c = *info;
        if (c >= 'a' && c <= 'z')
            c += 'A' - 'a';
        if (!(c >= 'A' && c <= 'Z'))
            return -1;
        set_last_letter(c);
        snprintf(message, CMD_ID_LEN + 1 + PLID_LEN + 1 + 1 + 1 + 2 + 2, "PLG %s %c %d\n", get_plid(), c, get_no_tries());
        return strnlen(message, CMD_ID_LEN + 1 + PLID_LEN + 1 + 1 + 1 + 2 + 2);
    } else if (is_guess(command, strnlen(command, MAX_COMMAND))) {
        if (get_plid() == NULL) {
            return -1;
        }
        snprintf(message, CMD_ID_LEN + 1 + PLID_LEN + 1 + WORD_MAX + 1 + 2 + 2, "PWG %s %s %d\n", get_plid(), info, get_no_tries());
        return strnlen(message, CMD_ID_LEN + 1 + PLID_LEN + 1 + WORD_MAX + 1 + 2 + 1);
    } else if (is_rev(command, strnlen(command, MAX_COMMAND))) {
        if (get_plid() == NULL) {
            return -1;
        }
        snprintf(message, CMD_ID_LEN + 1 + PLID_LEN + 2, "REV %s\n", get_plid());
        return strnlen(message, CMD_ID_LEN + 1 + PLID_LEN + 1);
    } else if (is_quit(command, strnlen(command, MAX_COMMAND))) {
        if (get_plid() == NULL) {
            return -1;
        }
        snprintf(message, QUT_MESSAGE_LEN + 1, "QUT %s\n", get_plid());
        return 1;
    } else if (is_exit(command, strnlen(command, MAX_COMMAND))) {
        if (get_plid() == NULL) {
            return -1;
        }
        snprintf(message, QUT_MESSAGE_LEN + 1, "QUT %s\n", get_plid());
        return 0;
    } else if (is_scoreboard(command, strnlen(command, MAX_COMMAND))) {
        char sc[5] = "GSB\n";
        strcpy(message, sc);
        return -strnlen(message, 5);
    } else if (is_hint(command, strnlen(command, MAX_COMMAND))) {
        if (get_plid() == NULL) {
            return -1;
        }
        snprintf(message, CMD_ID_LEN + 1 + PLID_LEN + 2, "GHL %s\n", get_plid());
        return -strnlen(message, CMD_ID_LEN + 1 + PLID_LEN + 1);
    } else if (is_state(command, strnlen(command, MAX_COMMAND))) {
        if (get_plid() == NULL) {
            return -1;
        }
        snprintf(message, CMD_ID_LEN + 1 + PLID_LEN + 2, "STA %s\n", get_plid());
        return strnlen(message, CMD_ID_LEN + 1 + PLID_LEN + 1);
    }
    return -1;
}

int is_letter(char c) {
    if (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z'))
        return TRUE;
    return FALSE;
}

int is_valid_fname(char *fname) {
    while (*fname != '\0' && *fname != '.') {
        if (!is_letter(*fname) && *fname != '-' && *fname != '_')
            return FALSE;
        fname++;
    }
    if (*fname == '\0')
        return FALSE;
    while (*fname != '\0') {
        if (!is_letter(*fname))
            return FALSE;
        fname++;
    }
    return TRUE;
}

/*
Receives command id and status. Returns the number of bytes read,
or -1 for errors.
*/
int parse_tcp_header(int fd, F_INFO *f) {
    char header[CMD_ID_LEN + 1 + STATUS_LEN + 2];
    char file_info[FNAME_LEN + 1 + FSIZE_LEN + 2];
    char cmd_id[CMD_ID_LEN + 1];
    char status[STATUS_LEN + 1];
    int n_var;
    ssize_t n;

    n = complete_read_tcp_header(fd, header);
    if (n == -1)
        return -1;
    n_var = sscanf(header, "%s %s", cmd_id, status);
    if (n_var < 1)
        return -1;
    if (is_r_scoreboard(cmd_id)) {
        if (is_st_empty(status))
            return 0;
        if (is_st_ok(status)) {
            n = complete_read_file_info(fd, file_info);
            if (n == -1)
                return -1;
            n_var = sscanf(file_info, "%s %ld", f->f_name, &(f->f_size));
            if (n_var != 2)
                return -1;
            if (!is_valid_fname(f->f_name))
                return -1;
            return n;
        }
    } else if (is_r_hint(cmd_id)) {
        if (is_st_ok(status)) {
            n = complete_read_file_info(fd, file_info);
            if (n == -1)
                return -1;
            n_var = sscanf(file_info, "%s %ld", f->f_name, &(f->f_size));
            if (n_var != 2)
                return -1;
            if (!is_valid_fname(f->f_name))
                return -1;
            return n;
        }
    } else if (is_r_state(cmd_id)) {
        if (is_st_act(status)) {
            n = complete_read_file_info(fd, file_info);
            if (n == -1)
                return -1;
            n_var = sscanf(file_info, "%s %ld", f->f_name, &(f->f_size));
            if (n_var != 2)
                return -1;
            if (!is_valid_fname(f->f_name))
                return -1;
            return n;
        }
        if (is_st_fin(status)) {
            n = complete_read_file_info(fd, file_info);
            if (n == -1)
                return -1;
            n_var = sscanf(file_info, "%s %ld", f->f_name, &(f->f_size));
            if (n_var != 2)
                return -1;
            if (!is_valid_fname(f->f_name))
                return -1;
            return n;
        }
        if (is_st_nok(status))
            return 0;
    }
    return -1;
}

int process_udp_response(char *response, int nbytes) {
    char buffer[100];
    char code[CMD_ID_LEN + 1];
    char status[STATUS_LEN + 1];
    int a, b, i, aux;
    int *pos;
    char *ptr;
    sscanf(response, "%s %s %d %d", code, status, &a, &b);
    if (is_r_start(code)) {
        if (is_st_ok(status)) {
            start_game(buffer, a, b);
            printf(buffer);
            return 0;
        } else if (is_st_nok(status)) {
            printf("Error: Unable to initialize a new game. Unfinished game already exists.\n");
        }
        return -1;
    } else if (is_r_play(code)) {
        if (is_st_ok(status)) {
            ptr = response + (CMD_ID_LEN + 1 + 2 + 1);
            if (a < 10)
                ptr += 1 + 1;
            else
                ptr += 2 + 1;
            if (b < 10)
                ptr += 1 + 1;
            else
                ptr += 2 + 1;
            pos = (int *)malloc(b * sizeof(int));
            for (i = 0; i < b; i++) {
                sscanf(ptr, "%d", &aux);
                pos[i] = aux;
                if (pos[i] < 10)
                    ptr += 1 + 1;
                else
                    ptr += 2 + 1;
            }
            play_game(buffer, b, pos);
            printf(buffer);
            return 0;
        } else if (is_st_win(status)) {
            win_game(buffer);
            printf(buffer);
            return 0;
        } else if (is_st_dup(status)) {
            printf("Error: Duplicate guess.\n");
        } else if (is_st_nok(status)) {
            printf("The letter doesn't exist in the word. You can only fail %d more time(s).\n", wrong_try());
        } else if (is_st_ovr(status)) {
            printf("GAME OVER!\n");
        } else if (is_st_inv(status)) {
            printf("Error: Invalid trial number.\n");
        } else if (is_st_err(status)) {
            printf("Error: Wrong play request.\n");
        }
        return -1;
    } else if (is_r_guess(code)) {

    } else if (is_r_quit(code)) {

    } else if (is_r_rev(code)) {
    }
    return -1;
}

int process_udp_message(char *response, char *message) {
    char *m_ptr = message;
    char cmd_id[CMD_ID_LEN + 1];
    char plid[PLID_LEN + 1];
    char word[WORD_MAX + 1];
    char c;
    int trial;

    printf("Message: '%s'\n", message);

    if (sscanf(m_ptr, "%s %s", cmd_id, plid) != 2)
        return -1;
    if (!is_id(plid, strlen(plid)))
        return -1;
    m_ptr += CMD_ID_LEN + 1 + PLID_LEN;
    if (is_m_start(cmd_id)) {
        printf("%d\n",*m_ptr);
        if (*m_ptr != '\n')
            return -1;
        start_new_game(response, plid);
    } else if (is_m_play(cmd_id)) {
        if (sscanf(m_ptr, "%c %d", &c, &trial) != 2)
            return -1;
        m_ptr += 1 /*c*/ + 1 /* */ + 1 /*digit*/ + 1 /*\n*/;
        if (trial > 10)
            m_ptr += 1 /*extra_digit*/;
        if (*m_ptr != '\n')
            return -1;
        play_letter_guess(response, plid, c, trial);
    } else if (is_m_guess(cmd_id)) {
        if (sscanf(m_ptr, "%s %d", word, &trial) != 2)
            return -1;
        m_ptr += strlen(word) + 1 /* */ + 1 + /*digit*/ + 1 /*\n*/;
        if (trial > 10)
            m_ptr += 1 /*extra_digit*/;
        if (*m_ptr != '\n')
            return -1;
        play_word_guess(response, plid, word, trial);
    } else if (is_m_quit(cmd_id)) {
        if (*m_ptr != '\n')
            return -1;
        quit(response, plid);
    } else if (is_m_rev(cmd_id)) {
        if (*m_ptr != '\n')
            return -1;
        rev(response, plid);
    } else {
        return -1;
    }
    return 0;
}

int process_tcp_message(char *response, char *message) {
    return -1;
}
