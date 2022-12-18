#include "common.h"

ssize_t complete_write(int fd, char *buffer, ssize_t n);
ssize_t complete_read(int fd, char *buffer, ssize_t n);
ssize_t complete_read_word(int fd, char *buffer, ssize_t max);
ssize_t complete_read_tcp_header(int fd, char *buffer);
ssize_t complete_read_file_info(int fd, char *buffer);
ssize_t complete_read_to_file(int src, FILE *dest, ssize_t n);