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
    hints.ai_flags = AI_PASSIVE;

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

    if (listen(fd, 5) == -1) {
        write(2, "ERROR: Listen was not successful.\n", 34);
        exit(1);
    }

    while (1) {
        addrlen=sizeof(addr);
        if ((newfd=accept(fd, (struct sockaddr*)&addr, &addrlen)) == -1) {
            write(2, "ERROR: Accept was not successful.\n", 34);
            exit(1);
        }

        n = read(newfd, buffer, 128);
        if (n == -1) {
            write(2, "ERROR: Message was not received correctly.\n", 43);
            exit(1);
        }
        write(1, "received: ", 10);
        write(1, buffer, n);

        n = write(newfd, buffer, n);
        if (n == -1) {
            write(2, "ERROR: Message was not sent to buffer.\n", 39);
            exit(1);
        }

        close(newfd);
    }
    freeaddrinfo(res);
    close(fd);
    return 0;
}