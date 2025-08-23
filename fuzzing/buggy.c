#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
	char buf1[16];
    char buf[100];
	char buf2[16];

    if (argc < 2) {
        printf("Usage: %s <input>\n", argv[0]);
        return 0;
    }

    strcpy(buf, argv[1]);

    if (strcmp(buf, "secret123") == 0) {
        printf("You found the secret!\n");
    } else {
        printf("Input was: %s\n", buf);
    }

    return 0;
}

