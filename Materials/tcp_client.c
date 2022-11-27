#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#define PORT "58001"

int main() {
    int fd, errcode, newfd;
    ssize_t n;
    socklen_t addrlen;
    struct addrinfo hints, *res;
    struct sockaddr_in addr;
    char buffer[128];

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        write(2, "ERROR: Socket was not created correctly.\n", 41);
        exit(1);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    errcode = getaddrinfo(NULL, PORT, &hints, &res);
    if (errcode != 0) {
        write(2, "ERROR: Address information was not received.\n", 45);
        exit(1);
    }

    n = connect(fd, res->ai_addr,res->ai_addrlen);
    if (n == -1) {
        write(2, "ERROR: Connection was not sucessful.\n", 37);
        exit(1);
    }

    n = write(fd, "Hello World!\n", 13);
    if (n == -1) {
        write(2, "ERROR: Message was not written to socket.\n", 42);
        exit(1);
    }

    n = read(fd, buffer,128);
    if (n == -1) {
        write(2, "ERROR: Message was not received from socket.\n", 45);
        exit(1);
    }

    write(1, "message: ", 9); 
    write(1, buffer,n);

    freeaddrinfo(res);
    close(fd);
    return 0;
}