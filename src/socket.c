#include "socket.h"

#define BLOCK_SIZE 1024

ssize_t complete_write(int fd, char *buffer, ssize_t n) {
    ssize_t nleft = n;
    ssize_t nwritten = 0;
    char *ptr = buffer;

    while (nleft > 0) {
        nwritten = write(fd, ptr, nleft);
        if (nwritten <= 0)
            return -1;
        nleft -= nwritten;
        ptr += nwritten;
        if (*(ptr - 1) == '\n')
            break;
    }
    return n - nleft;
}

ssize_t complete_read(int fd, char *buffer, ssize_t n) {
    ssize_t nleft = n;
    ssize_t nread = 0;
    char *ptr = buffer;

    while (nleft > 0) {
        nread = read(fd, ptr, nleft);
        if (nread == -1)
            return -1;
        nleft -= nread;
        ptr += nread;
        if (nread == 0 || *(ptr-1) == '\n')
            break;
    }
    return n - nleft;
}

ssize_t complete_read_word(int fd, char *buffer, ssize_t max) {
    ssize_t nleft = max;
    ssize_t nread = 0;
    char *ptr = buffer;
    while (nleft > 0) {
        nread = read(fd, ptr, 1);
        if (nread == -1)
            return -1;
        nleft -= nread;
        if (*ptr == '\n' || *ptr == ' ')
            return max - nleft;
        ptr += nread;
    }
    return max - nread;
}

ssize_t complete_read_tcp_header(int fd, char *buffer) {
    ssize_t nread = 0, ntotal = 0;
    char *ptr = buffer;

    nread = complete_read_word(fd, ptr, CMD_ID_LEN + 1);
    ptr += nread;
    ntotal += nread;
    if (*(ptr - 1) == '\n')
        return ntotal;
    nread = complete_read_word(fd, ptr, STATUS_LEN + 1);
    ptr += nread;
    ntotal += nread;
    return ntotal;
}

ssize_t complete_read_file_info(int fd, char *buffer) {
    ssize_t nread = 0, ntotal = 0;
    char *ptr = buffer;

    nread = complete_read_word(fd, ptr, FNAME_LEN + 1);
    ptr += nread;
    ntotal += nread;
    if (*(ptr - 1) == '\n')
        return -1;
    nread = complete_read_word(fd, ptr, FSIZE_LEN + 1);
    ptr += nread;
    ntotal += nread;
    if (*(ptr - 1) == '\n')
        return -1;
    return ntotal;
}

ssize_t complete_read_to_file(int src, FILE *dest, ssize_t n) {
    char buffer[BLOCK_SIZE + 1];
    ssize_t nleft = n + 1;
    ssize_t nread = 0;

    while (nleft > 0) {
        if (nleft >= BLOCK_SIZE)
            nread = complete_read(src, buffer, BLOCK_SIZE);
        else
            nread = complete_read(src, buffer, nleft);
        if (nread == -1)
            return -1;
        else if (nread == 0)
            break;
        fwrite(buffer, sizeof(char), nread, dest);
        nleft -= nread;
    }
    fflush(dest);
    return n - nleft;
}

ssize_t complete_write_file_to_socket(FILE *src, int dest, ssize_t n) {
    char buffer[BLOCK_SIZE + 1];
    ssize_t nleft = n;
    ssize_t nwrote = 0;

    while (nleft > 0) {
        if (nleft >= BLOCK_SIZE)
            nwrote = fread(buffer, sizeof(char), BLOCK_SIZE, src);
        else
            nwrote = fread(buffer, sizeof(char), nleft, src);
        if (nwrote == -1)
            return -1;
        else if (nwrote == 0)
            break;
        complete_write(dest, buffer, nwrote);
        nleft -= nwrote;
    }
    complete_write(dest, "\n", sizeof(char));
    return n - nleft + 1;
}