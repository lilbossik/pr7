#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

volatile sig_atomic_t guessed = 0;
volatile sig_atomic_t guess = 0;

void handle_sigusr1(int sig) {
    guessed = 1; // Угадал
}

void handle_sigusr2(int sig) {
    guessed = 2; // Не угадал
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <N>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    if (N <= 0) {
        fprintf(stderr, "N должно быть положительным числом.\n");
        return 1;
    }

    signal(SIGUSR1, handle_sigusr1);
    signal(SIGUSR2, handle_sigusr2);

    for (int i = 0; i < 10; i++) { // 10 циклов
        pid_t pid = fork();
        if (pid < 0) {
            perror("Ошибка fork");
            exit(1);
        }

        if (pid == 0) { // Второй игрок
            srand(time(NULL));
            while (1) {
                printf("Введите ваше предположение (от 1 до %d): ", N);
                scanf("%d", &guess);
                kill(getppid(), SIGUSR2); // Предположение не угадано

                if (guess == N) { // Угадал
                    kill(getppid(), SIGUSR1);
                    break;
                }
            }
            exit(0);
        } else { // Первый игрок
            int secret_number = rand() % N + 1;
            printf("Первый игрок загадал число: %d\n", secret_number);

            while (1) {
                pause(); // Ожидание сигнала от второго игрока

                if (guessed == 1) {
                    printf("Второй игрок угадал число %d!\n", secret_number);
                    break;
                } else if (guessed == 2) {
                    printf("Попытка угадать число %d не удалась.\n", guess);
                }
                guessed = 0; // Сброс состояния
            }
            wait(NULL); // Ожидание завершения второго процесса
        }
    }

    return 0;
}
