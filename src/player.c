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
    char buf[MAXCOM];
    char IPv4_addr[INET_ADDRSTRLEN];
    char * hostname = DEFAULT_HOSTNAME; // !! free after use
    char port[MAXPORT];
    int udp_socket, tcp_socket;
    struct addrinfo hints, *udp_addr, *tcp_addr;
    struct in_addr *addr;

    memcpy(port, DEFAULT_PORT, D_PORT_LEN);

    if (parse_cli(argc, argv, &hostname, port, &verbose) == -1) {
        fprintf(stderr, "[ERROR] Parsing command line parameters.\n");
        exit(1);
    }
    if (verbose) printf("host: %s\nport: %s\nverbose: %d\n", hostname, port, verbose);
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_CANONNAME;

    if (getaddrinfo(hostname, port, &hints, &udp_addr) != 0) {
        fprintf(stderr, "[ERROR] Getting UDP address information.\n");
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
    }
    if (verbose) {
        addr = &((struct sockaddr_in *)tcp_addr->ai_addr)->sin_addr;
        printf("%s IPv4 address for TCP connections: %s\n", tcp_addr->ai_canonname, inet_ntop(tcp_addr->ai_family, addr, IPv4_addr, sizeof(IPv4_addr)));
    }

    while (1) {
        if ((d = parse_input(buf)) == -1){
            exit(1);
        }
        
    }
    // d = -1 -> erro no input -> continue
    // d = 0 -> exit app -> enviar mensagem + sair da app
    // d = 1 -> reset game state -> enviar mensagem + reset + continue


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

