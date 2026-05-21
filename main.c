#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

#define MAX_INPUT 256
#define MAX_ARGS 64

int main() {
    char input[MAX_INPUT];
    char *args[MAX_ARGS];
    char *args_r[MAX_ARGS];

    while (1) {
        putchar('\n');

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
        char *pipePos = strchr(input, '|');
        if (pipePos != NULL) {
            int count = 0;
            int index = 0;
            while(input[index] != '\0') {
                if (input[index] == '|') {
                    count++;
                }
                index++;
            }
            if (count > 1) {
                printf("myshell: only support 1 pipe rn -_-\n");
                continue;
            }
            char *left;
            char *right;

            *pipePos = '\0';

            char *end = pipePos - 1;
            while(end > input && isspace((unsigned char)*end)) {
                *end = '\0';
                end--;
            }

            pipePos++;
            while (*pipePos == ' ') pipePos++;

            left = input;
            right = pipePos;
            
            char *ptr_l;
            char *ptr_r;

            args[i] = strtok_r(left, " ", &ptr_l);

            while(args[i] != NULL && i < MAX_ARGS - 1) {
                i++;
                args[i] = strtok_r(NULL, " ", &ptr_l);
            }
            args[i] = NULL;
            
            i = 0;

            args_r[i] = strtok_r(right, " ", &ptr_r);

            while (args_r[i] != NULL && i < MAX_ARGS - 1) {
                i++;
                args_r[i] = strtok_r(NULL, " ", &ptr_r);
            }
            args_r[i] = NULL;

            int fd[2];
            pipe(fd);

            pid_t p1 = fork();

            if (p1 == 0) {
                dup2(fd[1], STDOUT_FILENO);
                close(fd[0]);
                close(fd[1]);

                execvp(args[0], args);
                perror("myshell");
                exit(1);
            }

            pid_t p2 = fork();

            if(p2 == 0) {
                dup2(fd[0], STDIN_FILENO);
                close(fd[1]);
                close(fd[0]);

                execvp(args_r[0], args_r);
                perror("myshell");
                exit(1);
            }

            close(fd[0]);
            close(fd[1]);

            waitpid(p2, NULL, 0);
            waitpid(p1, NULL, 0);

            continue;
        }

        i = 0;
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
            waitpid(pid, NULL, 0);
        } else {
            perror("myshell");
        }
    }

    return 0;
}
