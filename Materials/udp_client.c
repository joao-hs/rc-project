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
    
    errcode = getaddrinfo("tejo.tecnico.ulisboa.pt", PORT, &hints, &res);
    if (errcode != 0) {
        write(2, "ERROR: Address information was not received.\n", 45);
        exit(1);
    }

    n = sendto(fd, "Hello World!\n", 13, 0, res->ai_addr, res->ai_addrlen); // Server is waiting 
    if (n == -1) {
        write(2, "ERROR: Message was not sent.\n", 29);
        exit(1);
    }

    addrlen = sizeof(addr);
    n = recvfrom(fd, buffer, 128, 0, (struct sockaddr*)&addr, &addrlen); // Waiting response from server
    if (n == -1) {
        write(2, "ERROR: Message was not received\n", 32);
        exit(1);
    }

    write(1, "echo: ", 6);
    write(1, buffer, n);
    freeaddrinfo(res);
    close(fd);
    return 0;
}