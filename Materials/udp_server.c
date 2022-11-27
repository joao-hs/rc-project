#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#define PORT "58001"

int main() {
    int fd, errcode;
    ssize_t n;
    socklen_t addrlen;
    struct addrinfo hints, *res;
    struct sockaddr_in addr;
    char buffer[128];

    fd = socket(AF_INET, SOCK_DGRAM, 0); // UDP socket
    if (fd == -1) {
        write(2, "ERROR: Socket was not created correctly.\n", 41);
        exit(1);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4 ; use AF_INET6 for IPv6
    hints.ai_socktype = SOCK_DGRAM; // UDP socket
    hints.ai_flags = AI_PASSIVE; // the returned socket will be suitable for binding

    errcode = getaddrinfo(NULL, PORT, &hints, &res);
    if (errcode != 0) {
        write(2, "ERROR: Address information was not received.\n", 45);
        exit(1);
    }

    n = bind(fd, res->ai_addr, res->ai_addrlen);
    if (n == -1) {
        write(2, "ERROR: Bind was not successful.\n", 32);
        exit(1);
    }

    while (1) {
        addrlen = sizeof(addr);
        n = recvfrom(fd, buffer, 128, 0, (struct sockaddr*)&addr, &addrlen); // Blocks until datagram is received from a client
        if (n == -1) {
            write(2, "ERROR: Message was not received\n", 32);
            exit(1);
        }
        write(1, "received: ", 10);
        write(1, buffer, n);

        n = sendto(fd, buffer, n, 0, (struct sockaddr*)&addr, addrlen); // Sends echo to client
        if (n == -1) {
            write(2, "ERROR: Message was not sent.\n", 29);
            exit(1);
        }
    }
    freeaddrinfo(res);
    close(fd);
}