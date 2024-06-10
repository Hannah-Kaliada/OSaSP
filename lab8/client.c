#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>


#define BUFFER_SIZE 100

int serverDescriptor;

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void cutNewLine(char *str) {
    int len = strlen(str);
    for (int i = 0; i < len; i++) {
        if (str[i] == '\n') {
            str[i] = '\0';
            break;
        }
    }
}

void sendCommandToServer(char* command) {
    if (send(serverDescriptor, command, BUFFER_SIZE, 0) == -1) {
        perror("Error sending command");
        close(serverDescriptor);
        exit(EXIT_FAILURE);
    }
}

void getResponseFromServer(char* buffer) {

    ssize_t bytesRead = recv(serverDescriptor, buffer, BUFFER_SIZE, 0);
    if (bytesRead == -1) {
        perror("Error receiving response.");
        close(serverDescriptor);
        exit(EXIT_FAILURE);
    }
    else if (bytesRead == 0) {
        perror("Server closed connection.");
        close(serverDescriptor);
        exit(EXIT_SUCCESS);
    }
}

void setConnection(int portNumber, struct hostent *server) {
    struct sockaddr_in serverAddress;

    serverDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (serverDescriptor < 0) error("Error opening socket");

    bzero((char *)&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serverAddress.sin_addr.s_addr, server->h_length);
    serverAddress.sin_port = htons(portNumber);

    if (connect(serverDescriptor, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
        error("Error connecting");
}

void listDirectory(char* command) {
    sendCommandToServer(command);
    char buffer[BUFFER_SIZE];
    while (1) {
        getResponseFromServer(buffer);
        if (strncmp(buffer, "list done", 9) == 0)
            break;
        printf("%s\n", buffer);
        bzero(buffer, BUFFER_SIZE);
    }
}

void changeDirectory(char* currentDir, char* command) {
    sendCommandToServer(command);
    char buffer[BUFFER_SIZE];
    getResponseFromServer(buffer);
    if (strncmp(buffer, "Error", 5) == 0) {
        printf("%s\n", buffer);
    } else {
        strcpy(currentDir, buffer);
    }
}

void getInfoFile() {
    char buffer[BUFFER_SIZE];
    while (1) {
        getResponseFromServer(buffer);
        if (strncmp(buffer, "info done", 9) == 0)
            break;
        cutNewLine(buffer);
        printf("%s\n", buffer);
        bzero(buffer, BUFFER_SIZE);
    }
}

void executeCommandsFromFile(char* fileName, char* currentDirectory) {

    FILE *file = fopen(fileName, "r");
    if (file == NULL) {
        fprintf(stderr, "ERROR opening file\n");
        return;
    }
    printf("Executing commands from file %s\n", fileName);

    char command[BUFFER_SIZE];
    char outputBuffer[BUFFER_SIZE];
    while (fgets(command, BUFFER_SIZE-1, file) != NULL) {
        cutNewLine(command);
        printf("Executing command: %s\n", command);
        bzero(outputBuffer, BUFFER_SIZE);
        if (strncmp(command, "info", 4) == 0) {
            sendCommandToServer(command);
            getInfoFile(serverDescriptor);
        } else if (strncmp(command, "list", 4) == 0) {
            listDirectory(command);
        } else if (strncmp(command, "cd", 2) == 0) {
            changeDirectory(currentDirectory, command);
        } else {
            sendCommandToServer(command);
            getResponseFromServer(outputBuffer);
            printf("%s\n", outputBuffer);
        }
        bzero(command, BUFFER_SIZE);
    }
    fclose(file);
}

void interactWithServer() {
    char command[BUFFER_SIZE];
    char outputBuffer[BUFFER_SIZE];
    bzero(outputBuffer, BUFFER_SIZE);
    char currentDirectory[BUFFER_SIZE];
    getResponseFromServer(currentDirectory);
    getInfoFile();

    while (1) {
        printf("%s> ", currentDirectory);
        bzero(command, BUFFER_SIZE);
        bzero(outputBuffer, BUFFER_SIZE);
        fgets(command, BUFFER_SIZE-1, stdin);
        cutNewLine(command);
        if (strncmp(command, "@", 1) == 0) {
            executeCommandsFromFile(command + 1, currentDirectory);
        } else if (strncmp(command, "info", 4) == 0) {
            sendCommandToServer(command);
            getInfoFile();
        } else if (strncmp(command, "list", 4) == 0) {
            listDirectory(command);
        } else if (strncmp(command, "cd", 2) == 0) {
            changeDirectory(currentDirectory, command);
        } else if(strncmp(command, "quit", 4) == 0){
            sendCommandToServer(command);
            getResponseFromServer(outputBuffer);
            printf("%s\n", outputBuffer);
            break;
        } else {
            sendCommandToServer(command);
            getResponseFromServer(outputBuffer);
            printf("%s\n", outputBuffer);
        }
    }
}

void handleSigint(int sig) {
    printf("\nClient terminating...\n");
    if (serverDescriptor != -1) {
        send(serverDescriptor, "quit", 4, 0);
        char buffer[BUFFER_SIZE];
        getResponseFromServer(buffer);
        printf("%s\n", buffer);
        close(serverDescriptor);
    }
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <hostName> <portNum>\n", argv[0]);
        exit(0);
    }

    int portNumber = atoi(argv[2]);
    struct hostent *server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "Error: non-existent host\n");
        exit(0);
    }
    setConnection(portNumber, server);
    signal(SIGINT, handleSigint);

    interactWithServer();

    close(serverDescriptor);
    return 0;
}
