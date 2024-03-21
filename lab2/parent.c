#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_ENV_VAR_LENGTH 100

extern char **environ;

void print_sorted_environment();
void launch_child_process(char symbol);

int main() {
    char symbol;

    // Set LC_COLLATE to C
    setenv("LC_COLLATE", "C", 1);

    while (1) {
        print_sorted_environment();

        printf("Enter '+' to launch child process using getenv(), '*' to use main() parameters, '&' to use extern char **environ, or 'q' to quit: ");
        scanf(" %c", &symbol);

        if (symbol == 'q') {
            printf("Exiting parent process.\n");
            break;
        }

        launch_child_process(symbol);
    }

    return 0;
}

void print_sorted_environment() {
    int i, j;
    int env_count = 0;
    char *env[MAX_ENV_VAR_LENGTH];

    // Copying environment variables
    for (i = 0; environ[i] != NULL; i++) {
        env[env_count++] = strdup(environ[i]);
    }

    // Sorting environment variables
    for (i = 0; i < env_count - 1; i++) {
        for (j = i + 1; j < env_count; j++) {
            if (strcmp(env[i], env[j]) > 0) {
                char *temp = env[i];
                env[i] = env[j];
                env[j] = temp;
            }
        }
    }

    // Printing sorted environment variables
    printf("\nSorted Environment Variables:\n");
    for (i = 0; i < env_count; i++) {
        printf("%s\n", env[i]);
        free(env[i]);
    }
}

void launch_child_process(char symbol) {
    char *args[2];
    args[1] = NULL;

    switch (symbol) {
        case '+':
            args[0] = "+";
            break;
        case '*':
            args[0] = "*";
            break;
        case '&':
            args[0] = "&";
            break;
        default:
            printf("Invalid input.\n");
            return;
    }

    // Forking and executing child process
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        execve("/. child", args, environ);
        perror("execve");
        exit(EXIT_FAILURE);
    } else {
        int status;
        waitpid(pid, &status, 0);
    }
}
