#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

extern char **environ; // Глобальная переменная, содержащая окружение процесса

// Функция для создания новой строки, обрезанной по указанному символу 'c'
char* substring(const char* str, const char c) {
    for (int i = 0; ; i++) {
        if (str[i] == c) {
            // Выделение памяти для новой строки
            char* ret = malloc((i + 1) * sizeof(char));
            int j;
            // Копирование символов до указанного символа 'c'
            for (j = 0; j < i; j++) {
                ret[j] = str[j];
            }
            ret[j] = '\0'; // Добавление завершающего нулевого символа
            return ret;
        }
        if (str[i] == '\0') {
            return NULL; // Возврат NULL, если символ 'c' не найден в строке
        }
    }
}

// Функция возвращает указатель на символ в строке после указанного символа 'c'
char* stringAfterCharacter(char* str, char c) {
    for (int i = 0; ; i++) {
        if (str[i] == c) {
            return str + i + 1; // Возврат указателя на символ после 'c'
        }
    }
}

// Функция для поиска значения параметра в массиве окружения
const char* findValueForParam(char** array, const char* key) {
    for (int i = 0; array[i] != NULL; i++) {
        const char* name = substring(array[i], '='); // Получение имени параметра
        if (name != NULL) {
            if (strcmp(name, key) == 0) { // Если найдено совпадение имени
                const char* value = stringAfterCharacter(array[i], '='); // Получение значения параметра
                return value;
            }
        }
    }
    return NULL; 
}

int main(int argc, char *argv[], char* envp[]) {

    fprintf(stdout, "%s process begins...\n", argv[0]); // Вывод имени программы

    if (argc != 3) {
        printf("Error in child Program params!");
        exit(3); // Выход с кодом ошибки, если параметры программы указаны неверно
    }

    char mode = argv[2][0]; // Режим работы программы

    pid_t pid = getpid(); // Получение идентификатора текущего процесса
    pid_t ppid = getppid(); // Получение идентификатора родительского процесса

    FILE *File = fopen(argv[1],"r"); // Открытие файла для чтения

    printf ("pid : %d\n", (int)pid); // Вывод идентификатора текущего процесса
    printf ("ppid : %d\n", (int)ppid); // Вывод идентификатора родительского процесса

    if (File == NULL) {
        printf("File %s not open\n", argv[1]);
        exit(-1); // Выход с кодом ошибки, если файл не удалось открыть
    }

    char bufParam[256]; // Буфер для считывания параметров из файла

    // Чтение параметров из файла и их обработка в соответствии с выбранным режимом
    while (fgets(bufParam, 255, File) != NULL) {
        if (bufParam[strlen(bufParam)-1]=='\n')
            bufParam[strlen(bufParam)-1] = '\0'; // Удаление символа новой строки

        switch (mode) {
            case '+': { // Режим '+' - вывод значений переменных окружения
                printf("%-40s %s\n", bufParam, getenv(bufParam));
                break;
            }
            case '*': { // Режим '*' - поиск значений параметров в переданных в программу переменных окружения
                const char* value = findValueForParam(envp, bufParam);
                if (value != NULL)
                    printf("%-40s %s\n", bufParam, value);
                else
                    printf("%-40s [Not Found]\n", bufParam); 
                break;
            }
            case '&': { // Режим '&' - поиск значений параметров в системных переменных окружения
                const char* value = findValueForParam(environ, bufParam);
                if (value != NULL)
                    printf("%-40s %s\n", bufParam, value);
                else
                    printf("%-40s [Not Found]\n", bufParam); 
                break;
            }
            default: {
                break;
            }
        }
    }
    fclose(File);
    exit(0);
}
