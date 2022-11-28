#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>

struct explained_addrinfo {   // (item in a linked list)
    int                 ai_flags;           // additional options
    int                 ai_family;          // address family
    int                 ai_socktype;        // socket type
    int                 ai_protocol;        // protocol
    socklen_t           ai_addrlen;         // address length (bytes)
    struct sockaddr     *ai_addr;           // socket address
    char                *ai_canonname;      // canonical hostname
    struct addrinfo     *ai_next;           // next item
};

struct explained_sockaddr_in {
    sa_family_t         sin_family;         // address family: AF_INET
    u_int16_t           sin_port;           // port in (16 bits) 
    struct in_addr      sin_addr;           // internet address
};

struct explained_in_addr {
    uint32_t            s_addr;             // 32 bits
};

uint32_t ntohl(uint32_t netlong); // (Network TO Host Long)
// Network byte order -> Big Endian


int main(void) {
    struct addrinfo hints, *res, *p;
    int errcode;
    char buffer[INET_ADDRSTRLEN];
    struct in_addr *addr;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;

    if ((errcode = getaddrinfo("other pc", NULL, &hints, &res)) != 0)
        fprintf(stderr, "error: getaddrinfo: %s\n", gai_strerror(errcode));
    else {
        printf("canonical hostname: %s\n", res->ai_canonname);
        for (p=res; p != NULL; p = p->ai_next) {
            addr = &((struct sockaddr_in *)p->ai_addr)->sin_addr;
            printf("internet address: %s (%08lX)\n",
                inet_ntop(p->ai_family, addr, buffer, sizeof(buffer)),
                (long unsigned int)ntohl(addr->s_addr));
        }
    freeaddrinfo(res);
    }
    exit(0);
    return 1;
}