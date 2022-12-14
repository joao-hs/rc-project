#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>

int main() {
    char in_str[128];
    printf("%.06d\n", 9924922);
    exit(1);
    fd_set inputs, testfds;
    struct timeval timeout;
    int i, out_fds, n, errcode;
    FD_ZERO(&inputs);
    FD_SET(0, &inputs);
    printf("Size of fd set: %d\n", sizeof(fd_set));
    printf("Value of FD_SETSIZE: %d\n", FD_SETSIZE);
    while(1) {
        testfds = inputs;
        memset((void *)&timeout, 0, sizeof(timeout));
        timeout.tv_sec=10;
        out_fds = select(FD_SETSIZE, &testfds, (fd_set *)NULL, (fd_set *)NULL, (struct timeval *) &timeout);
        switch (out_fds) {
            case 0:
                printf("Timeout event\n");
                break;
            case -1:
                perror("select");
                exit(1);
            default:
                if (FD_ISSET(0, &testfds)) {
                    fgets(in_str, 128, stdin);
                    printf("Input: %s", in_str);
                }
        }
    }

    return 0;
}