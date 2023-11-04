#include <sys/personality.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
const char *flag = "flag";
static void aslr_off(char *argv[])
{
        char *aslr;
        aslr = getenv("ASLR");
        if (NULL == aslr) {
                int rc;
                char *path;
                struct stat statbuf;
                rc = personality(ADDR_NO_RANDOMIZE);
                if (-1 == rc) {
                        perror("could␣not␣turn␣off␣ASLR");
                        exit(EXIT_FAILURE);
                }
                rc = setenv("ASLR", "off", true);
                if (-1 == rc) {
                        perror("error␣updating␣environment");
                        exit(EXIT_FAILURE);
                }
                rc = stat("/proc/self/exe", &statbuf);
                if (-1 == rc) {
                        /* Probably in chroot jail. */
                        path = "./service-nop";
                } else {
                        /* Probably running locally. */
                        path = "/proc/self/exe";
                }
                execv(path, argv);
                perror("could␣not␣exec");
                exit(EXIT_FAILURE);
        }
}
static void read_a_bit(void)
{
        /* NOTE: bigger buffer than shellcode project! */
        char buffer[2048] = {0};
        printf("filename:␣%0.8zx\n", flag); /* A leak! */
        fgets(buffer, 4096, stdin);
}
int main(int argc, char *argv[])
{
        setvbuf(stdout, NULL, _IONBF, 0);
        setvbuf(stdin, NULL, _IONBF, 0);
        aslr_off(argv);
        read_a_bit();
        exit(EXIT_SUCCESS);
}