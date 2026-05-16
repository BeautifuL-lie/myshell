#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#define MAX_INPUT 256
#define MAX_ARGS 64

int main() {
    char input[MAX_INPUT];
    char *args[MAX_ARGS];

    while (1) {
        char workdir[256];
        getcwd(workdir, sizeof(workdir));

        printf("%s\n", workdir);
        printf("~> ");
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
        char *saveptr;
        args[i] = strtok_r(input, " ", &saveptr);

        while(args[i] != NULL && i < MAX_ARGS - 1) {
            i++;
            args[i] = strtok_r(NULL, " ", &saveptr);
        }

        args[i] = NULL;

        // Built-in CD
        if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL || strcmp(args[1], "~") == 0) {
                if (chdir(getenv("HOME")) != 0) {
                    printf("cd: HOME environment not set\n");
                }
            } else {
                if (chdir(args[1]) != 0) {
                    perror("cd");
                }
            }
            continue;
        }
        
        //Built-in exec
        if (strcmp(args[0], "exec") == 0) {
            if (args[1] == NULL) {
                printf("exec needs argument!\n");
                continue;
            }

            execvp(args[1], args + 1);
            perror("myshell");
            continue;
        }

        pid_t pid = fork();

        if (pid == 0) {
            execvp(args[0], args);
            perror("myshell");
            exit(1);
        } else if (pid > 0) {
            wait(NULL);
        } else {
            perror("myshell");
        }
    }

    return 0;
}
