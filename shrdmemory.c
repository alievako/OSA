/*
2.1. One wants to transfer data between two processes.
2.1.1. To this aim, we use a shared memory segment of size MAX bytes.
2.1.2. When the emitter wants to send n bytes of data, it places them at the beginning of the
segment, then sends a message containing n to the receiver through a message
queue.
2.1.3. When the receiver completes the reception of the message, it sends a message to
the emitter to confirm that the memory location can be freed.
Program both processes.
*/

#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#define MAX 1024

struct message_buffer
{
    long msg_type;
    long msg_data;
};

void remove_ipc(int, int, int);
void writer(int, int);
void reader(int, long *);
int readline(char[], int);
void parent(int, int, char *);
void child(int, int, char *);

int main()
{

    int msg_id1, msg_id2, shm_id;
    msg_id1 = msgget(IPC_PRIVATE, 0600);
    msg_id2 = msgget(IPC_PRIVATE, 0600);
    shm_id = shmget(IPC_PRIVATE, MAX, 0600);
    if (msg_id1 == -1 || msg_id2 == -1)
    {
        perror("msgget");
        exit(EXIT_FAILURE);
    }
    if (shm_id == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    char *shmaddr = shmat(shm_id, NULL, 0);

    switch (fork())
    {
    case -1:
        perror("fork");
        exit(EXIT_FAILURE);
    case 0:
        child(msg_id1, msg_id2, shmaddr);
        exit(EXIT_SUCCESS);
    default:
        parent(msg_id1, msg_id2, shmaddr);
        exit(EXIT_SUCCESS);
    }
    remove_ipc(msg_id1, msg_id2, shm_id);
    return 0;
}

void remove_ipc(int msg_id1, int msg_id2, int shm_id)
{
    if (msg_id1 != -1)
    {
        msgctl(msg_id1, IPC_RMID, NULL);
    }
    if (msg_id2 != -1)
    {
        msgctl(msg_id2, IPC_RMID, NULL);
    }
    if (shm_id != -1)
    {
        shmctl(shm_id, IPC_RMID, NULL);
    }
}
int readline(char text[], int n_bytes)
{
    int r, length;
    if (fgets(text, n_bytes, stdin) == NULL)
    {
        r = EOF;
    }
    else
    {
        length = strlen(text);
        if (length > 0 && text[length - 1] == '\n')
            text[length - 1] = '\0';
        r = !EOF; //r=0
    }
    return r;
}
void writer(int msg_id, int n_bytes)
{
    struct message_buffer m;
    m.msg_type = 1;
    m.msg_data = n_bytes;
    if (msgsnd(msg_id, &m, n_bytes, 0) == -1)
    {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }
}
void reader(int msg_id, long *data)
{
    struct message_buffer m;
    if (msgrcv(msg_id, &m, MAX, 0, 0) == -1)
    {
        perror("msgrcv");
        exit(EXIT_FAILURE);
    }
    *data = m.msg_data; //msg_data will be assigned to the address that data points to
}

void parent(int msg_id1, int msg_id2, char *shmaddr)
{
    long data;
    while (readline(shmaddr, MAX) != EOF)
    {
        puts("writing to queue...");
        writer(msg_id1, strlen(shmaddr));
        puts("reading...");
        reader(msg_id2, &data);
    }
    writer(msg_id1, 0);
}
void child(int msg_id1, int msg_id2, char *shmaddr)
{
    long length;
    reader(msg_id1, &length);
    if (length > 0)
    {
        printf("The data stored in shared memory: %s\n", shmaddr);
        printf("The data received from queue: %ld\n", length);
    }
    writer(msg_id2, 0);
}
