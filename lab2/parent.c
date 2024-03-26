#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>
#include <locale.h>

extern char **environ; // Глобальная переменная, содержащая окружение процесса

char currentChildName[] = "child_00\0"; // Начальное имя для дочернего процесса

// Функция сравнения строк для использования в qsort
int compare(const void *a, const void *b) {
    return strcoll(*(char**)a, *(char**)b);
}

// Функция для создания новой строки, обрезанной по указанному символу 'c'
char* substring(const char* str, const char c) {
    for (int i = 0; ; i++){
        if (str[i] == c){
            char* ret = malloc((i + 1) * sizeof(char));
            int j;
            for (j = 0; j < i; j++){
                ret[j] = str[j];
            }
            ret[j] = '\0';
            return ret;
        }
        if (str[i] == '\0'){
            return NULL;
        }
    }
}

// Функция возвращает указатель на символ в строке после указанного символа 'c'
char* stringAfterCharacter(char* str, char c) {
    for (int i = 0; ; i++){
        if (str[i] == c){
            return str + i + 1;
        }
    }
}

// Функция для поиска пути к исполняемому файлу дочернего процесса в массиве окружения
const char* findChildName(char** array, const char* key) {
    for (int i = 0; array[i] != NULL; i++) {
        const char* name = substring(array[i], '=');
        if (name != NULL) {
            if (strcmp(name, key) == 0) {
                const char* value = stringAfterCharacter(array[i], '=');
                return value;
            }
        }
    }
    return NULL;
}

// Функция для определения размера массива окружения
int getenvpSize(char* envp[]){
    int envpSize = 0;
    while (envp[envpSize] != NULL) {
        envpSize++;
    }
    return envpSize;
}

// Функция для инкрементирования имени дочернего процесса
void incrementName(){
    int len = strlen(currentChildName);
    if (currentChildName[len - 1] == '9'){
        currentChildName[len - 2]++;
        currentChildName[len - 1] = '0';
    }
    else currentChildName[len - 1]++;
}

// Функция для вывода окружения
void printenvp(char* envp[]){
    printf("Environment:\n");
    for (int i = 0; envp[i] != NULL; i++) {
        char* equal_sign = strchr(envp[i], '=');
        if (equal_sign != NULL) {
            *equal_sign = '\0'; // Разделение строки на ключ и значение
            printf("%-40s  %s\n", envp[i], equal_sign + 1); // Выравнивание по столбцам
            *equal_sign = '='; // Восстановление строки
        }
    }
}

// Функция для запуска дочернего процесса
void executeChild(char option, char* args[], char* envp[]){
    int childStatus;

    pid_t childPid = fork(); // Создание дочернего процесса
    if (childPid == -1) {
        printf("Error occurred");
        exit(errno);
    }
    if (childPid == 0){ // Код, выполняемый дочерним процессом
        switch (option) {
            case '+': {
                execve(getenv("CHILD_PATH"),args,envp); // Запуск программы по указанному пути
            }
            case '*': {
                execve(findChildName(envp, "CHILD_PATH"),args,envp); // Поиск пути к программе в окружении
            }
            case '&': {
                execve(findChildName(environ, "CHILD_PATH"),args,envp); // Поиск пути к программе в системных переменных окружения
            }
            default: {
                break;
            }
        }
    }
    waitpid(childPid, &childStatus, 0); // Ожидание завершения дочернего процесса
    printf("Child process has ended with %d exit status\n", childStatus);
}

int main(int argc, char* argv[], char* envp[]) {
    setlocale(LC_COLLATE,"C"); // Установка локали для сравнения строк

    if (argc != 2){
        printf ("Specify filename\n ");
        exit(2);
    }

    qsort(envp, getenvpSize(envp), sizeof(char*), compare); // Сортировка массива окружения

    printenvp(envp); // Вывод окружения

    char mode;
    char* args[] = {currentChildName, argv[1], &mode, NULL}; // Аргументы для запуска дочернего процесса

    while (1) {
        printf("Enter '+' to launch child process using getenv(), '*' to use main() parameters, '&' to use extern char **environ, or 'q' to quit\n");
        printf ("\nInput option:\n");
        char option = getchar();
        getchar(); // Очистка буфера ввода
        switch (option){
            case '+': {
                args[2][0] = '+';
                executeChild('+', args, envp);
                incrementName(); // Инкрементирование имени дочернего процесса
                break;
            }
            case '*': {
                args[2][0] = '*';
                executeChild('*', args, envp);
                incrementName(); // Инкрементирование имени дочернего процесса
                break;
            }
            case '&': {
                args[2][0] = '&';
                executeChild('&', args, envp);
                incrementName(); // Инкрементирование имени дочернего процесса
                break;
            }
            case 'q': {
                return 0;
            }
            default: {
                printf ("Error input\n");
                break;
            }
        }
    }
}
