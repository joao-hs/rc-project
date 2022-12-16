#include "common.h"

ssize_t complete_write(int fd, char * buffer, ssize_t n);
ssize_t complete_read(int fd, char * buffer, ssize_t n);
ssize_t complete_read_to_file(int src, int dest, ssize_t n);
ssize_t read_tcp_header(int fd, char * cmd_id, char * status, File * f_info);