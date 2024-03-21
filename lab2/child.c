#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_ENV_VAR_LENGTH 100

void print_child_info(char *argv[], char *env_mode);
void read_and_print_environment(char *env_mode);

int child_main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <environment_mode>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *env_mode = argv[1];

    print_child_info(argv, env_mode);
    read_and_print_environment(env_mode);

    return 0;
}

void print_child_info(char *argv[], char *env_mode) {
    printf("\nChild Process Info:\n");
    printf("Name: %s\n", argv[0]);
    printf("PID: %d\n", getpid());
    printf("PPID: %d\n", getppid());
    printf("Environment Mode: %s\n", env_mode);
}

void read_and_print_environment(char *env_mode) {
    char *env_var;

    printf("\nEnvironment Variables:\n");
    if (strcmp(env_mode, "+") == 0) {
        env_var = getenv("CHILD_PATH");
        printf("CHILD_PATH=%s\n", env_var);
    } else {
        extern char **environ;

        for (char **env = environ; *env != NULL; env++) {
            printf("%s\n", *env);
        }
    }
}
