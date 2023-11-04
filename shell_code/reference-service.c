#include <stdio.h>
#include <stdlib.h>
#include <time.h>
static void read_a_bit(void)
{
        char buffer[128] = {0};
        register long bp asm("rbp");
        printf("buffer:␣%0.16zx\n", buffer); /* A leak! */
        printf("bp:␣␣␣␣␣%0.16zx\n", bp);     /* A leak! */
        fgets(buffer, 1024, stdin);
}
int main(void)
{
        setvbuf(stdout, NULL, _IONBF, 0);
        setvbuf(stdin, NULL, _IONBF, 0);
        read_a_bit();
        exit(EXIT_SUCCESS);
}