#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <signal.h>

#define BUFFER_SIZE 100

struct ServerThreadArguments {
    int clientSocketDescriptor;
    char* rootPath;
    char* infoFileName;
};

struct Server {
    int serverSocketDescriptor;
    int *clientSockets;
    int clientCount;
    pthread_mutex_t clientMutex;
};

struct Server server;

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void logEvent(const char *event) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timestamp[100];
    strftime(timestamp, sizeof(timestamp)-1, "%Y.%m.%d-%H:%M:%S", t);
    printf("%s %s\n", timestamp, event);
    fflush(stdout);
}

void sendMessage(int clientSocket, char* command) {
    if (send(clientSocket, command, BUFFER_SIZE, 0) == -1) {
        perror("Error sending command");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }
}

void listFiles(int clientSocketDescriptor, char *currentDir) {
    DIR *dirStream;
    struct dirent *dir;
    dirStream = opendir(currentDir);
    if (dirStream) {
        char buffer[BUFFER_SIZE];
        while ((dir = readdir(dirStream)) != NULL) {
            struct stat fileStat;
            char fullPath[BUFFER_SIZE];
            snprintf(fullPath, sizeof(fullPath), "%s/%s", currentDir, dir->d_name);
            if (lstat(fullPath, &fileStat) == -1) {
                perror("lstat");
                continue;
            }
            bzero(buffer, sizeof(buffer));
            if (S_ISDIR(fileStat.st_mode)) {
                snprintf(buffer, sizeof(buffer), "%s/", dir->d_name);
            } else if (S_ISLNK(fileStat.st_mode)) {
                char target[BUFFER_SIZE];
                ssize_t len = readlink(fullPath, target, sizeof(target)-1);
                if (len != -1) {
                    target[len] = '\0';
                    snprintf(buffer, sizeof(buffer), "%s --> %s", dir->d_name, target);
                } else {
                    perror("readlink");
                    snprintf(buffer, sizeof(buffer), "%s (broken link)", dir->d_name);
                }
            } else {
                snprintf(buffer, sizeof(buffer), "%s", dir->d_name);
            }

            sendMessage(clientSocketDescriptor, buffer);
        }
        closedir(dirStream);

        if (send(clientSocketDescriptor, "list done", 9, 0) == -1) {
            perror("Error sending command");
            close(clientSocketDescriptor);
            exit(EXIT_FAILURE);
        }
    } else {
        perror("opendir");
        sendMessage(clientSocketDescriptor, "Error opening directory");
    }
}

void changeDirectory(int socketDescriptor, char *currentDir, const char *newDir) {
    char cwd[1024];
    if(getcwd(cwd, sizeof(cwd)) != NULL) {
        if(strcmp(cwd, currentDir) == 0) {
            if (strcmp(newDir, "..") == 0) {
                sendMessage(socketDescriptor, "Error: Path out of bounds");
                return;
            }
        }
    } else {
        error("getcwd() error");
    }

    char newPath[1024];
    if (strncmp(newDir, "/", 1) == 0) {
        snprintf(newPath, sizeof(newPath), "%s%s", currentDir, newDir);
    } else {
        snprintf(newPath, sizeof(newPath), "%s/%s", currentDir, newDir);
    }

    char resolvedPath[1024];
    if (realpath(newPath, resolvedPath) == NULL) {
        perror("realpath");
        sendMessage(socketDescriptor, "Error resolving path");
        return;
    }

    struct stat statbuf;
    if (stat(resolvedPath, &statbuf) != 0 || !S_ISDIR(statbuf.st_mode)) {
        sendMessage(socketDescriptor, "Error: Not a directory");
        return;
    }

    strcpy(currentDir, resolvedPath);
    sendMessage(socketDescriptor, currentDir);
}

void listInfoFile(int clientSocketDescriptor, char* fileName) {
    FILE *file = fopen(fileName, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file\n");
        return;
    }

    char buffer[BUFFER_SIZE];
    while (fgets(buffer, BUFFER_SIZE-1, file) != NULL) {
        sendMessage(clientSocketDescriptor, buffer);
        bzero(buffer, BUFFER_SIZE);
    }
    send(clientSocketDescriptor, "info done", 9, 0);
    fclose(file);
}

void *handleClient(void *args) {
    struct ServerThreadArguments *arguments = (struct ServerThreadArguments*)args;
    int clientSocketDescriptor = arguments->clientSocketDescriptor;
    char currentDir[BUFFER_SIZE];
    strcpy(currentDir, arguments->rootPath);
    char infoFileName[BUFFER_SIZE];
    strcpy(infoFileName, arguments->infoFileName);

    char buffer[BUFFER_SIZE];

    sendMessage(clientSocketDescriptor, currentDir);
    listInfoFile(clientSocketDescriptor, infoFileName);
    while (1) {
        bzero(buffer, BUFFER_SIZE);
        ssize_t n = recv(clientSocketDescriptor, buffer, BUFFER_SIZE, 0);
        if (n == -1) {
            error("Error reading from socket");
        }

        logEvent(buffer);

        if (strncmp(buffer, "echo", 4) == 0) {
            sendMessage(clientSocketDescriptor, buffer + 5);
        } else if (strncmp(buffer, "quit", 4) == 0) {
            sendMessage(clientSocketDescriptor, "BYE");
            break;
        } else if (strncmp(buffer, "info", 4) == 0) {
            listInfoFile(clientSocketDescriptor, arguments->infoFileName);
        } else if (strncmp(buffer, "list", 4) == 0) {
            listFiles(clientSocketDescriptor, currentDir);
        } else if (strncmp(buffer, "cd", 2) == 0) {
            changeDirectory(clientSocketDescriptor, currentDir, buffer + 3);
        } else {
            sendMessage(clientSocketDescriptor, "Unknown command");
        }
    }
    close(clientSocketDescriptor);
    free(arguments->rootPath);
    free(arguments->infoFileName);
    free(arguments);

    pthread_mutex_lock(&server.clientMutex);
    for (int i = 0; i < server.clientCount; ++i) {
        if (server.clientSockets[i] == clientSocketDescriptor) {
            server.clientSockets[i] = -1;
            break;
        }
    }
    pthread_mutex_unlock(&server.clientMutex);
    return NULL;
}

void setConnection(int* socketDescriptor, int portNumber) {
    struct sockaddr_in serverAddress;
    *socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (*socketDescriptor < 0) {
        error("Error opening socket");
    }

    int optVal = 1;
    if (setsockopt(*socketDescriptor, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal)) < 0) {
        perror("Error: Could not set SO_REUSEADDR option\n");
    }

    bzero((char *)&serverAddress, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(portNumber);

    if (bind(*socketDescriptor, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        error("Error on binding");
    }
    listen(*socketDescriptor, 100);
}

void handleSigint(int sig) {
    logEvent("Server shutting down...");
    close(server.serverSocketDescriptor);
    pthread_mutex_lock(&server.clientMutex);
    for (int i = 0; i < server.clientCount; ++i) {
        if (server.clientSockets[i] != -1) {
            close(server.clientSockets[i]);
        }
    }
    pthread_mutex_unlock(&server.clientMutex);
    free(server.clientSockets);
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: ./programName <portNumber> <serverRootPath> <infoFile>\n");
        exit(1);
    }

    int portNumber;
    socklen_t clientAddressLen;
    struct sockaddr_in clientAddress;
    clientAddressLen = sizeof(clientAddress);

    portNumber = atoi(argv[1]);

    server.clientCount = 100; // Максимальное количество клиентов
    server.clientSockets = malloc(server.clientCount * sizeof(int));
    for (int i = 0; i < server.clientCount; ++i) {
        server.clientSockets[i] = -1;
    }
    pthread_mutex_init(&server.clientMutex, NULL);

    setConnection(&server.serverSocketDescriptor, portNumber);
    logEvent("Server is ready.");

    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = handleSigint;
    sigemptyset(&act.sa_mask);
    sigaction(SIGINT, &act, NULL);

    char* rootPath = (char*)malloc(strlen(argv[2]) + 1);
    strcpy(rootPath, argv[2]);
    char* infoFileName = (char*)malloc(strlen(argv[3]) + 1);
    strcpy(infoFileName, argv[3]);

    while (1) {
        int newSocketDescriptor = accept(server.serverSocketDescriptor, (struct sockaddr *)&clientAddress, &clientAddressLen);
        if (newSocketDescriptor < 0) {
            error("Error on accept");
        }
        logEvent("Client connected");

        pthread_mutex_lock(&server.clientMutex);
        int i;
        for (i = 0; i < server.clientCount; ++i) {
            if (server.clientSockets[i] == -1) {
                server.clientSockets[i] = newSocketDescriptor;
                break;
            }
        }
        pthread_mutex_unlock(&server.clientMutex);

        if (i == server.clientCount) {
            logEvent("Too many clients connected. Connection refused.");
            close(newSocketDescriptor);
        } else {
            struct ServerThreadArguments *args = malloc(sizeof(struct ServerThreadArguments));
            args->clientSocketDescriptor = newSocketDescriptor;
            args->rootPath = (char*)malloc(strlen(rootPath) + 1);
            strcpy(args->rootPath, rootPath);
            args->infoFileName = (char*)malloc(strlen(infoFileName) + 1);
            strcpy(args->infoFileName, infoFileName);

            pthread_t clientThread;
            pthread_create(&clientThread, NULL, handleClient, (void *)args);
            pthread_detach(clientThread);
        }
    }
    close(server.serverSocketDescriptor);
    free(server.clientSockets);
    return 0;
}
