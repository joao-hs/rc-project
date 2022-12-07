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
    //int i, notex = 1;
    char buf[MAXCOM];
    char * hostname = DEFAULT_HOSTNAME; // !! free after use
    char port[MAXPORT];
    memcpy(port, DEFAULT_PORT, D_PORT_LEN);

    if (parse_cli(argc, argv, &hostname, port, &verbose) == -1) {
        fprintf(stderr, "[ERROR] Parsing command line parameters.\n");
        exit(1);
    }
    if (verbose) printf("host: %s\nport: %s\nverbose: %d\n", hostname, port, verbose);
    
    if(parse_input(buf) == -1){
        exit(1);
    }

    free(hostname);
    return 0;
}

