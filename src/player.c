#include "common.h"
#include "interface.h"
#include "socket.h"

#define DEFAULT_HOSTNAME NULL
#define DEFAULT_PORT "58011"
#define TIME_OUT 10

extern int errno;

/*
Parse command line arguments and set the variables accordingly
*/
int parse_clargs(int argc, char *argv[], char **hostname, char *port, int *verbose, int *timeout_and_resend) {
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
        case 't':
            *timeout_and_resend = FALSE;
            n++;
            break;
        default:
            return -1;
        }
    }
    return n;
}

int main(int argc, char *argv[]) {
    int verbose = FALSE; // !! [DEBUG]
    int timeout_and_resend = TRUE;
    char message[MAX_MESSAGE];
    char udp_response[MAX_UDP_RESPONSE];
    // char tcp_response[10];
    char IPv4_addr[INET_ADDRSTRLEN];
    char *hostname = DEFAULT_HOSTNAME; // !! free after use
    char port[MAX_PORT];
    int udp_socket, tcp_socket;
    int in_code, udp_code, tcp_code, exit_code=1;
    struct addrinfo hints, *udp_addr, *tcp_addr;
    struct in_addr *addr;
    struct timeval timeout;
    socklen_t addrlen;
    F_INFO recv_f;

    memcpy(port, DEFAULT_PORT, strlen(DEFAULT_PORT));
    recv_f.f_size = 0;
    recv_f.f_data = NULL;

    if (parse_clargs(argc, argv, &hostname, port, &verbose, &timeout_and_resend) == -1) {
        fprintf(stderr, "[ERROR] Parsing command line parameters.\n");
        exit(1);
    }
    if (verbose)
        printf("host: %s\nport: %s\nverbose: %d\n", hostname, port, verbose);

    if ((udp_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        fprintf(stderr, "[ERROR] Creating UDP socket.\n");
        exit(1);
    }
    if (timeout_and_resend) {
        memset((void *)&timeout, 0, sizeof(timeout));
        timeout.tv_sec = TIME_OUT;
        if (setsockopt(udp_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1) {
            fprintf(stderr, "ERROR: Unable to set timeout to socket.\n");
            exit(1);
        }
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if (getaddrinfo(hostname, port, &hints, &udp_addr) != 0) {
        fprintf(stderr, "[ERROR] Getting UDP address information.\n");
        exit(1);
    }
    if (verbose) {
        addr = &((struct sockaddr_in *)udp_addr->ai_addr)->sin_addr;
        printf("IPv4 address for UDP connections: %s\n", inet_ntop(udp_addr->ai_family, addr, IPv4_addr, sizeof(IPv4_addr)));
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(hostname, port, &hints, &tcp_addr) != 0) {
        fprintf(stderr, "[ERROR] Getting TCP address information.\n");
        exit(1);
    }
    if (verbose) {
        addr = &((struct sockaddr_in *)tcp_addr->ai_addr)->sin_addr;
        printf("IPv4 address for TCP connections: %s\n", inet_ntop(tcp_addr->ai_family, addr, IPv4_addr, sizeof(IPv4_addr)));
    }
    while (exit_code) {
        if ((in_code = parse_input(message)) == -1) {
            /* Handle incorrect input */
            continue;
        }
        if (verbose)
            printf("Input code: %d\nMessage: '%s'\n", in_code, message);
        if (in_code >= 0) { // send message via UDP
            do {
                if (in_code == 0){
                    exit_code = 0;
                    udp_code = sendto(udp_socket, message, QUT_MESSAGE_LEN, 0, udp_addr->ai_addr, udp_addr->ai_addrlen);
                }
                else if (in_code == 1)
                    udp_code = sendto(udp_socket, message, QUT_MESSAGE_LEN, 0, udp_addr->ai_addr, udp_addr->ai_addrlen);
                else
                    udp_code = sendto(udp_socket, message, in_code, 0, udp_addr->ai_addr, udp_addr->ai_addrlen);

                if (udp_code == -1) {
                    fprintf(stderr, "[ERROR] Sending message to server.\n");
                    exit(1);
                }

                addrlen = sizeof(addr);
                udp_code = recvfrom(udp_socket, udp_response, MAX_UDP_RESPONSE, 0, (struct sockaddr *)&addr, &addrlen);
                if (udp_code == -1 && !timeout_and_resend) {
                    fprintf(stderr, "[ERROR] Receiving message from server.\n");
                    exit(1);
                }
            } while (timeout_and_resend && udp_code == -1);
            if (verbose) {
                *(udp_response + udp_code) = '\0';
                printf("Response: '%s'\n", udp_response);
            }
            process_udp_response(udp_response, udp_code, &exit_code);
        } else { // send message via TCP
            tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
            tcp_code = connect(tcp_socket, tcp_addr->ai_addr, tcp_addr->ai_addrlen);
            if (tcp_code == -1) {
                fprintf(stderr, "[ERROR] Establishing connection to server.\n");
                exit(1);
            }

            if ((tcp_code = complete_write(tcp_socket, message, -in_code)) == -1) {
                fprintf(stderr, "[ERROR] Sending message to server.\n");
                exit(1);
            }

            if ((tcp_code = parse_tcp_header(tcp_socket, &recv_f)) == -1) {
                fprintf(stderr, "[ERROR] 'Error' response or wrong format.\n");
                exit(1);
            }
            if (verbose)
                printf("File name: %s\nFile size: %ld\n", recv_f.f_name, recv_f.f_size);
            if (tcp_code > 0) {
                recv_f.f = fopen(recv_f.f_name, "w");
                if ((tcp_code = complete_read_to_file(tcp_socket, recv_f.f, recv_f.f_size)) != recv_f.f_size) {
                    fprintf(stderr, "[ERROR] Incomplete or corrupted writing.\n");
                    exit(1);
                }
                fclose(recv_f.f);
            }
            close(tcp_socket);
        }
    }

    freeaddrinfo(udp_addr);
    freeaddrinfo(tcp_addr);
    free(hostname);
    close(udp_socket);
    return 0;
}
