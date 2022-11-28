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

ssize_t complete_write(int fd, char * buffer, ssize_t n);
ssize_t complete_read(int fd, char * buffer, ssize_t n);

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
    int listener, errcode, newfd, ret;
    ssize_t n;
    socklen_t addrlen;
    struct addrinfo hints, *res;
    struct sockaddr_in addr;
    char buffer[128];
    struct sigaction act;
    pid_t pid;

    if (argc == 2) {
        if (strcmp("-v", argv[1]) == 0) verbose = TRUE;
    }

    /** Ignore SIGPIPE signals */
    if (verbose) printf("Ignoring SIGPIPE signals\n");
    memset(&act, 0, sizeof(act));
    act.sa_handler = SIG_IGN;
    if (sigaction(SIGPIPE, &act, NULL) == -1) {
        write(2, "ERROR: Could not set to ignore SIGPIPE signal.\n", 47);
        exit(1);
    }
    
    /** Avoid zombies when child processes die */
    if (verbose) printf("Avoiding zombies when child processes die\n");
    if (sigaction(SIGCHLD, &act, NULL) == -1) {
        write(2, "ERROR: Could not set to ignore SIGCHLD signal.\n", 47);
        exit(1);
    }

    /** Create socket */
    if (verbose) printf("Creating socket\n");
    listener = socket(AF_INET, SOCK_STREAM, 0); // TCP socket
    if (listener == -1) {
        write(2, "ERROR: Socket was not created correctly.\n", 41);
        exit(1);
    }

    /** Setup Hints */
    if (verbose) printf("Setting up Hints\n");
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4 ; use AF_INET6 for IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP socket
    hints.ai_flags = AI_PASSIVE; // the returned socket will be suitable for binding

    /** Get address info */
    if (verbose) printf("Getting address info\n");
    errcode = getaddrinfo(NULL, PORT, &hints, &res);
    if (errcode != 0) {
        write(2, "ERROR: Address information was not received.\n", 45);
        exit(1);
    }

    /** Bind local host to listener */
    if (verbose) printf("Binding local host to listener\n");
    n = bind(listener, res->ai_addr, res->ai_addrlen);
    if (n == -1) {
        write(2, "ERROR: Bind was not successful.\n", 32);
        exit(1);
    }

    /** Prepare listener to queue at most 5 connection requests */
    if (verbose) printf("Prepare listener to queue at most 5 connection requests\n");
    if (listen(listener, 5) == -1) {
        write(2, "ERROR: Listen was not successful.\n", 34);
        exit(1);
    }

    freeaddrinfo(res);
    if (verbose) printf("Entering main loop\n");
    while (1) {
        addrlen=sizeof(addr);
        if (verbose) printf("Accepting connection requests\n");
        do {
            newfd = accept(listener, (struct sockaddr*)&addr, &addrlen);
        } while (newfd == -1 && errno == EINTR);

        if (newfd == -1) {
            write(2, "ERROR: Accept was not successful.\n", 34);
            exit(1);
        }
        if (verbose) printf("Connection accepted\n");
        if (verbose) printf("Forking\n");
        pid = fork();
        if (pid == -1) {
            write(2, "ERROR: Fork was not successful.\n", 32);
            exit(1);
        }
        else if (pid == 0) { // Child process
            if (verbose) printf("[CHILD] Closing listener\n");
            close(listener); // Child is not supposed to listen to new connection requests
            if (verbose) printf("[CHILD] Reading from socket\n");
            if ((n = complete_read(newfd, buffer, 128)) == -1) {
                write(2, "ERROR: Message was not received correctly.\n", 43);
                exit(1);
            }
            write(1, "received: ", 10);
            write(1, buffer, n);
            if (verbose) printf("[CHILD] Writing echo\n");
            if ((n = complete_write(newfd, buffer, n)) == -1) {
                write(2, "ERROR: Message was not sent to buffer.\n", 39);
                exit(1);
            }
            close(newfd);
            if (verbose) printf("[CHILD] Completed\n");
            exit(0);
        }
        // Parent process
        /* if ((n = complete_read(newfd, buffer, 128)) == -1) {
            write(2, "ERROR: Message was not received correctly.\n", 43);
            exit(1);
        }
        write(1, "received: ", 10);
        write(1, buffer, n);

        if ((n = complete_write(newfd, buffer, n)) == -1) {
            write(2, "ERROR: Message was not sent to buffer.\n", 39);
            exit(1);
        } */
        if (verbose) printf("[PARENT] Waiting to close new socket\n");
        while ((ret = close(newfd)) == -1 && errno == EINTR);
        if (ret == -1) {
            write(2, "ERROR: Closing the child socket.\n", 33);
            exit(1);
        }
        if (verbose) printf("[PARENT] New socket closed\n");
    }
    close(listener);
    exit(0);
    return 0;
}