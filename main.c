#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

#define MAX_INPUT 256
#define MAX_ARGS 64

char *built_in_cmd[] = {"exit", "cd", "exec", NULL};

void printprompt() {
    putchar('\n');

    char workdir[256];
    getcwd(workdir, sizeof(workdir));

    printf("%s\n", workdir);
    printf("~> ");
    fflush(stdout);
}

void getinput(char *buf) {
    if (fgets(buf, MAX_INPUT, stdin) == NULL) {
        exit(0);
    }

    buf[strcspn(buf, "\n")] = '\0';
}

int containpipe(char *input) {
    int count = 0;
    int index = 0;

    while(input[index] != '\0') {
        if (input[index] == '|') {
            count++;
        }
        index++;
    }

    return count;
}

void parsepipe(char *input, char **args1, char **args2) {
    char *pipe_pos = strchr(input, '|');
    *pipe_pos = '\0';

    char *end = pipe_pos - 1;
    while(end > input && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }

    pipe_pos++;
    while (*pipe_pos == ' ') pipe_pos++;

    char *left = input;
    char *right = pipe_pos;

    char *ptr_l;
    char *ptr_r;

    int i = 0;

    args1[i] = strtok_r(left, " ", &ptr_l);

    while(args1[i] != NULL && i < MAX_ARGS - 1) {
        i++;
        args1[i] = strtok_r(NULL, " ", &ptr_l);
    }
    args1[i] = NULL;
    
    i = 0;

    args2[i] = strtok_r(right, " ", &ptr_r);

    while(args2[i] != NULL && i < MAX_ARGS - 1) {
        i++;
        args2[i] = strtok_r(NULL, " ", &ptr_r);
    }
    args2[i] = NULL;
}

void execpipe(char **args1, char **args2) {
    int fd[2];
    pipe(fd);

    pid_t p1, p2;

    p1 = fork();

    if (p1 == 0) {
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);

        execvp(args1[0], args1);
        perror("myshell");
        exit(1);
    }

    p2 = fork();

    if (p2 == 0) {
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);
        close(fd[1]);

        execvp(args2[0], args2);
        exit(1);
    }

    close(fd[0]);
    close(fd[1]);

    waitpid(p1, NULL, 0);
    waitpid(p2, NULL, 0);
}

void parseinput(char *input, char **args) {
    int i = 0;
    char *saveptr;
    args[i] = strtok_r(input, " \t\r\n", &saveptr);

    while(args[i] != NULL && i < MAX_ARGS - 1) {
        i++;
        args[i] = strtok_r(NULL, " \t\r\n", &saveptr);
    }

    args[i] = NULL;
}

int isbuiltincmd(char *args) {
    int i = 0;
    
    while (built_in_cmd[i] != NULL) {
        if (strcmp(built_in_cmd[i], args) == 0) {
            return 1;
        }
        i++;
    }
    return 0;
}

void expvar(char** args) {
    int i = 0;
    char *str;
    char *env;

    while(args[i] != NULL) {
        str = strchr(args[i], '$');
        if (str != NULL) {
            str++;
            env = getenv(str);
            if (env != NULL) {
                args[i] = env;
            }
        }
        i++;
    }
}

void execbuiltin(char **args) {
    //EXIT
    if (strcmp(args[0], "exit") == 0) exit(0);

    //CD
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
        return;
    }

    //EXEC
    if (strcmp(args[0], "exec") == 0) {
        if (args[1] == NULL) {
            printf("exec needs argument!\n");
            return;
        }

        execvp(args[1], args + 1);
        perror("myshell");
        return;
    }   
}

void execcmd (char **args) {
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

int main() {
    char input[MAX_INPUT];
    char *args1[MAX_ARGS];
    char *args2[MAX_ARGS];

    while (1) {
        printprompt();
        getinput(input);

        if (strlen(input) == 0) {
            continue;
        }

        int pipe_n = containpipe(input);
        
        if (pipe_n == 1) {
            parsepipe(input, args1, args2);
            expvar(args1);
            expvar(args2);
            execpipe(args1, args2);
            continue;
        } else if (pipe_n > 1) {
            printf("myshell: only support 1 pipe rn -_-\n");
            continue;
        }
        
        parseinput(input, args1);
        expvar(args1);
        if (args1[0] == NULL) {
            continue;
        }

        if (isbuiltincmd(args1[0])) {
            execbuiltin(args1);
            continue;
        }

        execcmd(args1);
    }

    return 0;
}
