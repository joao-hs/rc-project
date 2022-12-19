#include "common.h"
#include "interface.h"
#include "socket.h"
#include "game.h"

#define DEFAULT_HOSTNAME NULL
#define DEFAULT_PORT "58011"
#define D_HOST_LEN sizeof DEFAULT_HOSTNAME
#define D_PORT_LEN sizeof DEFAULT_PORT

extern int errno;

int main(int argc, char *argv[]) {
    int verbose = FALSE; // !! [DEBUG]
    char message[MAX_MESSAGE];
    char udp_response[MAX_UDP_RESPONSE];
    // char tcp_response[10];
    char IPv4_addr[INET_ADDRSTRLEN];
    char *hostname = DEFAULT_HOSTNAME; // !! free after use
    char port[MAX_PORT];
    int udp_socket, tcp_socket;
    int in_code, udp_code, tcp_code;
    struct addrinfo hints, *udp_addr, *tcp_addr;
    struct in_addr *addr;
    socklen_t addrlen;
    F_INFO recv_f;

    memcpy(port, DEFAULT_PORT, D_PORT_LEN);
    recv_f.f_size = 0;
    recv_f.f_data = NULL;

    if (parse_cli(argc, argv, &hostname, port, &verbose) == -1) {
        fprintf(stderr, "[ERROR] Parsing command line parameters.\n");
        exit(1);
    }
    if (verbose)
        printf("host: %s\nport: %s\nverbose: %d\n", hostname, port, verbose);

    if ((udp_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        fprintf(stderr, "[ERROR] Creating UDP socket.\n");
        exit(1);
    }

    if ((tcp_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "[ERROR] Creating TCP socket.\n");
        exit(1);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_CANONNAME; // ? maybe there's no need

    if (getaddrinfo(hostname, port, &hints, &udp_addr) != 0) {
        fprintf(stderr, "[ERROR] Getting UDP address information.\n");
        exit(1);
    }
    if (verbose) {
        addr = &((struct sockaddr_in *)udp_addr->ai_addr)->sin_addr;
        printf("%s IPv4 address for UDP connections: %s\n", udp_addr->ai_canonname, inet_ntop(udp_addr->ai_family, addr, IPv4_addr, sizeof(IPv4_addr)));
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME; // ? maybe there's no need

    if (getaddrinfo(hostname, port, &hints, &tcp_addr) != 0) {
        fprintf(stderr, "[ERROR] Getting TCP address information.\n");
        exit(1);
    }
    if (verbose) {
        addr = &((struct sockaddr_in *)tcp_addr->ai_addr)->sin_addr;
        printf("%s IPv4 address for TCP connections: %s\n", tcp_addr->ai_canonname, inet_ntop(tcp_addr->ai_family, addr, IPv4_addr, sizeof(IPv4_addr)));
    }
    // Para tirar
    //char plid[PLID_LEN];
    int trial = 0;
    // end
    while (1) {
        if ((in_code = parse_input(message, trial)) == -1) {
            /* Handle incorrect input */
            exit(1);
        }
        if (verbose)
            printf("Input code: %d\nMessage: '%s'\n", in_code, message);
        if (in_code >= 0) { // send message via UDP
            if (in_code == 0)
                udp_code = sendto(udp_socket, message, QUT_MESSAGE_LEN, 0, udp_addr->ai_addr, udp_addr->ai_addrlen);
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
            if (udp_code == -1) {
                fprintf(stderr, "[ERROR] Receiving message from server.\n");
                exit(1);
            }
            if (verbose) {
                *(udp_response + udp_code) = '\0';
                printf("Response: '%s'\n", udp_response);
            }
            process_udp_response(udp_response, udp_code);
        } else { // send message via TCP
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
        }
    }

    freeaddrinfo(udp_addr);
    freeaddrinfo(tcp_addr);
    free(hostname);
    close(udp_socket);
    close(tcp_socket);
    return 0;
}
