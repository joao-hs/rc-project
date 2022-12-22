#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef COMMON_H
#define COMMON_H

#define TRUE 1
#define FALSE 0
#define CMD_ID_LEN 3
#define STATUS_LEN 6
#define PLID_LEN 6
#define WORD_MIN 3
#define WORD_MAX 30
#define MAX_COMMAND 10
#define FNAME_LEN 24
#define FSIZE_LEN 10
#define FDATA_MAX 1073741824
#define MAX_PLID 6
#define MAX_HOST 253
#define MAX_PORT 6
#define MAX_MESSAGE CMD_ID_LEN + 1 + WORD_MAX + 1
#define MAX_UDP_RESPONSE CMD_ID_LEN + 1 + STATUS_LEN + 1 + 2 + 2 + WORD_MAX * 2
#define TCP_HEADER_RESPONSE CMD_ID_LEN + 1 + STATUS_LEN + 1 + FNAME_LEN + 1 + FSIZE_LEN + 1
#define MAX_TCP_RESPONSE TCP_HEADER_RESPONSE + FDATA_MAX + 1

#define MAX(x, y) ({__typeof__ (x) _x = (x); __typeof__ (y) _y = (y); _x > _y ? _x : _y; })
#define MIN(x, y) ({__typeof__ (x) _x = (x); __typeof__ (y) _y = (y); _x <= _y ? _x : _y; })

typedef struct file_info {
    FILE *f;
    char f_name[FNAME_LEN];
    ssize_t f_size;
    char *f_data;
} F_INFO;
#endif /* COMMON_H */