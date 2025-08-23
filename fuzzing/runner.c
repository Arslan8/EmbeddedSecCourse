#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef FORTIFY
#define TARGET "./buggy_fortify"
#else 
#define TARGET "./buggy"
#endif

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "r");
    if (!f) { perror("fopen"); return 1; }

    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = malloc(sz + 1);
    if (!buf) { perror("malloc"); fclose(f); return 1; }

    fread(buf, 1, sz, f);
    buf[sz] = '\0';
    fclose(f);

    // Strip a single trailing newline if present
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    }

    char *args[] = {TARGET, buf, NULL};
    execv(args[0], args);

    perror("execv"); // only reached on failure
    free(buf);
    return 1;
}

