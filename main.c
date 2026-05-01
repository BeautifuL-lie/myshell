#include <stdio.h>
#include <string.h>
#define MAX_INPUT 256
#define MAX_ARGS 64

int main() {
    char input[MAX_INPUT];
    char *args[MAX_ARGS];

    while (1) {
        printf("myshell> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0) {
            continue;
        }

        if (strcmp(input, "exit") == 0) {
            break;
        }

        int i = 0;
        args[i] = strtok(input, " ");

        while(args[i] != NULL && i < MAX_ARGS - 1) {
            i++;
            args[i] = strtok(NULL, " ");
        }

        args[i] = NULL;
    }

    return 0;
}
