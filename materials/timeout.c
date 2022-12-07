#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#define PORT "58001"
#define TRUE 1
#define FALSE 0

extern int errno;

ssize_t complete_write(int fd, char * buffer, ssize_t n) {
    ssize_t nleft = n;
    ssize_t nwritten = 0;
    char *ptr = buffer;

    while (nleft > 0) {
        nwritten = write(fd, ptr, nleft);
        if (nwritten <= 0) return -1;
        nleft -= nwritten;
        ptr += nwritten;
    }
    return n;
}

ssize_t complete_read(int fd, char * buffer, ssize_t n) {
    ssize_t nleft = n;
    ssize_t nread = 0;
    char *ptr = buffer;

    while (nleft > 0) {
        nread = read(fd, ptr, nleft);
        if (nread == -1) return -1;
        else if (nread == 0) return n - nleft;
        nleft -= nread;
        ptr += nread;
    }
    return n;
}


int main(int argc, char *argv[]) {
    int verbose = FALSE;
    int fd, errcode, newfd;
    ssize_t n;
    socklen_t addrlen;
    struct addrinfo hints, *res;
    struct sockaddr_in addr;
    char in_buffer[128], out_buffer[128];
    struct sigaction act;
    struct timeval timeout;


    if (argc == 2) {
        if (strcmp("-v", argv[1]) == 0) verbose = TRUE;
    }

    if (verbose) printf("Ignoring SIGPIPE signals\n");
    memset(&act, 0, sizeof(act));
    act.sa_handler = SIG_IGN;

    if (sigaction(SIGPIPE, &act, NULL) == -1) {
        write(2, "ERROR: Could not set to ignore SIGPIPE signal.\n", 47);
        exit(1);
    }

    if (verbose) printf("Creating socket\n");
    fd = socket(AF_INET, SOCK_STREAM, 0); // TCP socket
    if (fd == -1) {
        write(2, "ERROR: Socket was not created correctly.\n", 41);
        exit(1);
    }

    memset((void *)&timeout, 0, sizeof(timeout));
    timeout.tv_sec = 5;
    if (verbose) printf("Setting 5 seconds timeout to socket\n");
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1) {
        write(2, "ERROR: Unable to set timeout to socket.\n", 40);
        perror(strerror(errno));
        exit(1);
    }

    if (verbose) printf("Setting up Hints\n");
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4 ; use AF_INET6 for IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP socket

    if (verbose) printf("Getting address info\n");
    errcode = getaddrinfo(NULL, PORT, &hints, &res);
    if (errcode != 0) {
        write(2, "ERROR: Address information was not received.\n", 45);
        exit(1);
    }

    if (verbose) printf("Connection request to host\n");
    n = connect(fd, res->ai_addr,res->ai_addrlen); // Connection request to server
    if (n == -1) {
        write(2, "ERROR: Connection was not sucessful.\n", 37);
        exit(1);
    }

    if (verbose) printf("Writing message in host's socket\n");
    if ((n = complete_write(fd, "Hello World!\n", 13)) == -1) {
        write(2, "ERROR: Message was not written to socket.\n", 42);
        exit(1);
    }

    if (verbose) printf("Reading message from host's socket\n");
    if ((n = complete_read(fd, out_buffer, n)) == -1) {
        write(2, "ERROR: Message was not received from socket.\n", 45);
        exit(1);
    }

    write(1, "message: ", 9); 
    write(1, out_buffer, n);

    freeaddrinfo(res);
    close(fd);
    return 0;
}