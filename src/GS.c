#include "common.h"
#include "socket.h"
#include "games.h"
#include "interface.h"

#define PORT "58011"

int main(int argc, char * argv[]) {
    int listener, udp_socket, tcp_socket;
    socklen_t addrlen;
    struct addrinfo hints, *res;
    struct sockaddr_in addr;
    int udp_code, tcp_code;
    char message[MAX_MESSAGE];
    char response[MAX_MESSAGE];
    pid_t pid;

    

    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket == -1) {
        fprintf(stderr, "[ERROR] Creating UDP socket.\n");
    }

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener == -1) {
        fprintf(stderr, "[ERROR] Creating TCP listener socket.\n");
        exit(1);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    
    udp_code = getaddrinfo(NULL, PORT, &hints, &res);
    if (udp_code != 0) {
        fprintf(stderr, "[ERROR] Getting UDP local address information.\n");
        exit(1);
    }

    udp_code = bind(udp_socket, res->ai_addr, res->ai_addrlen);
    if (udp_code == -1) {
        fprintf(stderr, "[ERROR] Binding UDP socket.\n");
        exit(1);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    tcp_code = getaddrinfo(NULL, PORT, &hints, &res);
    if (tcp_code != 0) {
        fprintf(stderr, "[ERROR] Getting TCP local address information.\n");
        exit(1);
    }

    tcp_code = bind(listener, res->ai_addr, res->ai_addrlen);
    if (tcp_code == -1) {
        fprintf(stderr, "[ERROR] Binding TCP socket.\n");
        exit(1);
    }

    tcp_code = listen(listener, 10);
    if (tcp_code == -1) {
        fprintf(stderr, "[ERROR] Setting up listener socket to listen for at most 10 connection requests at the same time.\n");
        exit(1);
    }

    freeaddrinfo(res);
    pid = fork();
    if (pid == -1) {
        fprintf(stderr, "[ERROR] Forking to deal with UDP and TCP requests.\n");
        exit(1);
    }
    if (pid == 0) { // UDP
        while (1) {
            addrlen = sizeof(addr);
            udp_code = recvfrom(udp_socket, message, MAX_MESSAGE, 0, (struct sockaddr*)&addr, &addrlen);
            if (udp_code == -1) {
                fprintf(stderr, "[ERROR] Receiving message.\n");
                break;
            }
            printf("Received: ");
            write(1, message, udp_code);

            udp_code = process_udp_message(response, message);
            if (udp_code == -1) {
                fprintf(stderr, "[ERROR] Invalid message.\n");
                continue;
            }
            printf("Response: '%s'\n", response);
            udp_code = sendto(udp_socket, response, strlen(response), 0, (struct sockaddr*)&addr, addrlen);
            if (udp_code == -1) {
                fprintf(stderr, "[ERROR] Message was not sent.\n");
                continue;
            }
        }
    } else { // TCP
        while (1) {
            /* accept requests */
            addrlen = sizeof(addr);
            do {
                tcp_socket = accept(listener, (struct sockaddr*)&addr, &addrlen);
            } while (tcp_socket == -1);
            if (tcp_socket == -1) {
                fprintf(stderr, "[ERROR] Accepting new TCP connection request.\n");
                continue;
            }
            /* fork() */
            pid = fork();
            if (pid == -1) {
                fprintf(stderr, "[ERROR] Forking to deal with multiple TCP requests.\n");
                continue;
            }
            if (pid == 0) {
                close(listener);
                tcp_code = complete_read(tcp_socket, message, MAX_MESSAGE);
                if (tcp_code == -1) {
                    fprintf(stderr, "[ERROR] Message was not received correctly.\n");
                    exit(1);
                }
                tcp_code = process_tcp_message(message, response);
                if (tcp_code == -1) {
                    fprintf(stderr, "[ERROR] Invalid message.\n");
                    continue;
                }
                /* "Copy" file and "paste" in socket */
                close(tcp_socket);
                exit(0);
            } else {
                while ((tcp_code = close(tcp_socket)) == -1);
                if (tcp_code == -1) {
                    fprintf(stderr, "[ERROR] Closing child's socket.\n");
                    exit(1);
                }
            }
        }
    }
    return 0;
}