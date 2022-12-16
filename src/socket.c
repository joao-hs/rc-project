#include "socket.h"

#define BLOCK_SIZE 256

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

ssize_t complete_read_to_file(int src, int dest, ssize_t n) {
    char buffer[BLOCK_SIZE + 1];
    ssize_t nleft = n;
    ssize_t nread = 0;

    while (nleft > 0) {
        if (nleft >= BLOCK_SIZE)
            nread = read(src, buffer, BLOCK_SIZE);
        else
            nread = read(src, buffer, nleft);
        if (nread == -1) return -1;
        else if (nread == 0) break;
        *(buffer + nread) = '\0';
        if (write(dest, buffer, nread) != nread) return -1;
        printf(buffer);
        nleft -= nread;
    }
    fsync(dest);
    return n - nleft;
}