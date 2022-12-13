#include "common.h"
#include "interface.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <signal.h>

#include <stdio.h>

#define DEFAULT_HOSTNAME NULL
#define DEFAULT_PORT "58011"
#define D_HOST_LEN sizeof DEFAULT_HOSTNAME
#define D_PORT_LEN sizeof DEFAULT_PORT

extern int errno;

int main(int argc, char * argv[]) {
    int verbose = FALSE; // !! [DEBUG]
    char message[MAX_MESSAGE];
    char udp_response[MAX_UDP_RESPONSE];
    char tcp_response[MAX_TCP_RESPONSE];
    char IPv4_addr[INET_ADDRSTRLEN];
    char * hostname = DEFAULT_HOSTNAME; // !! free after use
    char port[MAX_PORT];
    int udp_socket, tcp_socket;
    int in_code, udp_code, tcp_code;
    struct addrinfo hints, *udp_addr, *tcp_addr;
    struct in_addr *addr;
    socklen_t addrlen;

    memcpy(port, DEFAULT_PORT, D_PORT_LEN);

    if (parse_cli(argc, argv, &hostname, port, &verbose) == -1) {
        fprintf(stderr, "[ERROR] Parsing command line parameters.\n");
        exit(1);
    }
    if (verbose) printf("host: %s\nport: %s\nverbose: %d\n", hostname, port, verbose);

    if (udp_socket = socket(AF_INET, SOCK_DGRAM, 0) == -1) {
        fprintf(stderr, "[ERROR] Creating UDP socket.\n");
        exit(1);
    }
    
    if (tcp_socket = socket(AF_INET, SOCK_STREAM, 0) == -1) {
        fprintf(stderr, "[ERROR] Creating TCP socket.\n");
        exit(1);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_CANONNAME;

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
    hints.ai_flags = AI_CANONNAME;

    if (getaddrinfo(hostname, port, &hints, &tcp_addr) != 0) {
        fprintf(stderr, "[ERROR] Getting TCP address information.\n");
        exit(1);
    }
    if (verbose) {
        addr = &((struct sockaddr_in *)tcp_addr->ai_addr)->sin_addr;
        printf("%s IPv4 address for TCP connections: %s\n", tcp_addr->ai_canonname, inet_ntop(tcp_addr->ai_family, addr, IPv4_addr, sizeof(IPv4_addr)));
    }
    //Para tirar
    int pid;
    // end
    while (1) {
        if ((in_code = parse_input(message, pid)) == -1){
            /* Handle incorrect input*/
            exit(1);
        }
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
            udp_code = recvfrom(udp_socket, message, (struct sockaddr*) &addr, &addrlen);
            if (udp_code == -1) {
                fprintf(stderr, "[ERROR] Receiving message from server.\n");
                exit(1);
            }
        }
        
    }


    /* send to server's socket (either UDP or TCP) */

    /* create thread to handle request: */
    /* if TCP connect + write */
    /* if UDP sendto */
    /* repeat */

    freeaddrinfo(udp_addr);
    freeaddrinfo(tcp_addr);
    free(hostname);
    return 0;
}

