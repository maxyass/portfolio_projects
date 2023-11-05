#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/time.h>

int main(int argc, char **argv) {
    int rc, fd;
    ssize_t size;
    char buf[1024];
    struct addrinfo hints = {.ai_socktype = SOCK_DGRAM};
    struct addrinfo *info = NULL, *p;
    int exit_code = EXIT_FAILURE;
    int next;

    if (argc != 4) {
        printf("usage: %s HOST START END\n", argv[0]);
        goto done;
    }
    int upper, lower;
    lower = atoi(argv[2]);
    upper = atoi(argv[3]);

    for (int i = lower; i < upper; i++) {
        next = 0;
        char port_str[6];
        snprintf(port_str, sizeof(port_str), "%d", i);
        rc = getaddrinfo(argv[1], port_str, &hints, &info);
        if (0 != rc) {
            continue; // Skip to the next port
        }

        // Create a socket
        for (p = info; p; p = p->ai_next) {
            if(next == 1) {
                break;
            }
            fd = socket(p->ai_family, p->ai_socktype, 0);
            if (-1 == fd) {
                continue;
            }

            // Set a timeout for recv
            struct timeval timeout;
            timeout.tv_sec = 0;
            timeout.tv_usec = 10000;
            if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
                continue;
            }

            // Send a UDP message; can be anything
            char message[17] = "Hello, UDP Port!";
            rc = sendto(fd, message, strlen(message), 0, p->ai_addr, p->ai_addrlen);
            if (rc == -1) {
                close(fd);
                continue;
            }

            // Receive response from the server
            for (;;) {
                size = recv(fd, buf, sizeof(buf), 0);
                if (size > 0) {
                    next = 1;
                    // Print the received data
                    write(STDOUT_FILENO, buf, size);
                    // Check for a newline character to determine the end of the message
                    if (strchr(buf, '\n') != NULL) {
                        break; // End of message
                    }
                } else if (size == -1) {
					next = 1;
                    break;
				} else {
                    // Handle a timeout error
                    next = 1;
                    break;
                }
            }

            close(fd); // Close the socket
        }
    }

    exit_code = EXIT_SUCCESS;

done:
    exit(exit_code);
}
