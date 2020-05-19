#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <errno.h>
#include <sys/wait.h>

int main()
{
    int i;
    key_t shm_key;
    int shm_id;
    int *shrd_value;
    pid_t pid;
    int fork_count;
    sem_t *sem;
    unsigned int sem_value;
    shm_key = ftok("/dev/null", 65);
    shm_id = shmget(shm_id, sizeof(int), 0644 | IPC_CREAT);
    if (shm_id < 0)
    {
        perror("shmgget");
        exit(EXIT_FAILURE);
    }
    shrd_value = shmat(shm_id, NULL, 0);
    *shrd_value = 0;
    puts("How many fork do you want?");
    scanf("%d", &fork_count);
    puts("Enter semaphore value");
    scanf("%u", &sem_value);
    sem = sem_open("semaphore", O_CREAT | O_EXCL, 0644, sem_value);

    for (i = 0; i < fork_count; i++)
    {
        pid = fork();
        if (pid < 0)
        {
            perror("fork");
            sem_unlink("semaphore");
            sem_close(sem);
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            break;
        }
    }
    if (pid == 0)
    {
        //child process
        sem_wait(sem);
        printf("Child - %d is in critical section\n", i);
        sleep(1);
        *shrd_value += i * 3;
        printf("Child - %d: new value of shrd_value = %d\n", i, *shrd_value);
        sem_post(sem); //release
        exit(EXIT_SUCCESS);
    }
    else if (pid > 0)
    {
        //parent process
        while (pid = waitpid(-1, NULL, 0))
        {
            if (errno == ECHILD)
            {
                break;
            }
        }
        puts("All children exited");
        shmdt(shrd_value);
        shmctl(shm_id, IPC_RMID, 0);
        sem_unlink("semaphore"); // removes named semaphore
        sem_close(sem);          // frees resources
        exit(0);
    }
}