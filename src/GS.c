#include "common.h"
#include "games.h"
#include "interface.h"
#include "socket.h"
#include "signal.h"

#define DEFAULT_PORT "58011"

extern int randomize;
int listener, udp_socket, tcp_socket;

void handler(int arg) {
    free_word_list();
    close(udp_socket);
    close(listener);
    close(tcp_socket);
    kill(0, SIGKILL);
    exit(1);
}

int parse_clargs(int argc, char *argv[], char *word_list, char *port, int *verbose, int *randomize) {
    int n = 0, i = 2;
    if (argc < 2) // Mandatory ./GS fname_word_list
        return -1;

    if (!is_valid_fname(argv[1]))
        return -1;

    strcpy(word_list, argv[1]);
    n++;

    for (; i < argc; i++) {
        if (argv[i][0] != '-')
            return -1;
        switch (argv[i][1]) {
        case 'p':
            if (strlen(argv[i + 1]) > MAX_PORT)
                argv[i + 1][MAX_PORT] = '\0';
            strncpy(port, argv[i + 1], MAX_HOST + 1);
            i++;
            n++;
            break;
        case 'v':
            *verbose = TRUE;
            n++;
            break;
        case 'r':
            *randomize = TRUE;
            n++;
            break;
        default:
            return -1;
        }
    }
    return n;
}

int main(int argc, char *argv[]) {
    int verbose = FALSE;
    socklen_t addrlen;
    struct addrinfo hints, *res;
    struct sockaddr_in addr;
    int udp_code, tcp_code;
    char port[MAX_PORT];
    char message[MAX_MESSAGE + 1];
    char response[TCP_HEADER_RESPONSE + 1];
    char word_list[FNAME_LEN + 1];
    pid_t pid;
    FILE *tcp_file;
    struct sigaction act;

    memcpy(port, DEFAULT_PORT, strlen(DEFAULT_PORT));

    signal(SIGINT, handler);

    memset(&act, 0, sizeof(act));
    act.sa_handler = SIG_IGN;
    if (sigaction(SIGPIPE, &act, NULL) == -1) {
        fprintf(stderr, "[ERROR] Setting to ignore SIGPIPE signals.\n");
        exit(1);
    }
    if (sigaction(SIGCHLD, &act, NULL) == -1) {
        fprintf(stderr, "[ERROR] Setting to ignore SIGCHLD signals.\n");
        exit(1);
    }

    if (parse_clargs(argc, argv, word_list, port, &verbose, &randomize) < 1) {
        fprintf(stderr, "[ERROR] Parsing command line parameters.\n");
        exit(1);
    }

    if (save_word_list(word_list) == -1) {
        fprintf(stderr, "[ERROR] Reading word list.\n");
        exit(1);
    }

    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket == -1) {
        fprintf(stderr, "[ERROR] Creating UDP socket.\n");
        exit(1);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    udp_code = getaddrinfo(NULL, port, &hints, &res);
    if (udp_code != 0) {
        fprintf(stderr, "[ERROR] Getting UDP local address information.\n");
        exit(1);
    }

    udp_code = bind(udp_socket, res->ai_addr, res->ai_addrlen);
    if (udp_code == -1) {
        fprintf(stderr, "[ERROR] Binding UDP socket.\n");
        exit(1);
    }

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener == -1) {
        fprintf(stderr, "[ERROR] Creating TCP listener socket.\n");
        exit(1);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    tcp_code = getaddrinfo(NULL, port, &hints, &res);
    if (tcp_code != 0) {
        fprintf(stderr, "[ERROR] Getting TCP local address information.\n");
        exit(1);
    }

    tcp_code = bind(listener, res->ai_addr, res->ai_addrlen);
    if (tcp_code == -1) {
        fprintf(stderr, "[ERROR] Binding TCP socket.\n");
        exit(1);
    }

    tcp_code = listen(listener, 10);
    if (tcp_code == -1) {
        fprintf(stderr, "[ERROR] Setting up listener socket to listen for at most 10 connection requests at the same time.\n");
        exit(1);
    }

    freeaddrinfo(res);
    pid = fork();
    if (pid == -1) {
        fprintf(stderr, "[ERROR] Forking to deal with UDP and TCP requests.\n");
        exit(1);
    }
    if (pid == 0) { /* Child handles UDP */
        while (1) {
            addrlen = sizeof(addr);
            udp_code = recvfrom(udp_socket, message, MAX_MESSAGE, 0, (struct sockaddr *)&addr, &addrlen);
            if (udp_code == -1) {
                fprintf(stderr, "[ERROR] Receiving message.\n");
                break;
            }
            *(message + udp_code) = '\0';
            if (verbose) printf(message);
            udp_code = process_udp_message(response, message);
            if (udp_code == -1) {
                sendto(udp_socket, "ERR\n", 4, 0, (struct sockaddr *)&addr, addrlen);
                fprintf(stderr, "[ERROR] Invalid message.\n");
                continue;
            } else if (udp_code == 0) {
                continue;
            }
            printf("Response: '%s'\n", response);
            udp_code = sendto(udp_socket, response, strlen(response), 0, (struct sockaddr *)&addr, addrlen);
            if (udp_code == -1) {
                fprintf(stderr, "[ERROR] Message was not sent.\n");
                continue;
            }
        }
    } else { /* Parent handles TCP */
        while (1) {
            /* accept requests */
            addrlen = sizeof(addr);
            do {
                tcp_socket = accept(listener, (struct sockaddr *)&addr, &addrlen);
            } while (tcp_socket == -1);
            if (tcp_socket == -1) {
                fprintf(stderr, "[ERROR] Accepting new TCP connection request.\n");
                continue;
            }
            /* fork() */
            pid = fork();
            if (pid == -1) {
                fprintf(stderr, "[ERROR] Forking to deal with multiple TCP requests.\n");
                continue;
            }
            if (pid == 0) { /* Child handles the request */
                close(listener);
                tcp_code = complete_read(tcp_socket, message, MAX_MESSAGE);
                if (tcp_code == -1) {
                    fprintf(stderr, "[ERROR] Message was not received correctly.\n");
                    exit(1);
                }
                if (verbose) printf(message);
                tcp_code = process_tcp_message(response, message, &tcp_file);
                if (tcp_code == -1) {
                    fprintf(stderr, "[ERROR] Invalid message.\n");
                    continue;
                }
                
                complete_write(tcp_socket, response, strlen(response));
                if (tcp_code > 0) {
                    complete_write_file_to_socket(tcp_file, tcp_socket, tcp_code);
                    fclose(tcp_file);
                }
                close(tcp_socket);
                exit(0);
            } else { /* Parent accepts other upcomming requests */
                while ((tcp_code = close(tcp_socket)) == -1);
                if (tcp_code == -1) {
                    fprintf(stderr, "[ERROR] Closing child's socket.\n");
                    exit(1);
                }
            }
        }
    }
    close(listener);
    close(udp_socket);
    free_word_list();
    return 0;
}