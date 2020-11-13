#include <stdio.h>
#include "safe_ops.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

int main()
{
    int id_sem = semget(ftok("NOT_DOING_AN", 1), 1, IPC_CREAT | 0777);
    struct sembuf var;
    var.sem_num = 0;
    var.sem_op = -1;
    var.sem_flg = 0;

    int id = shmget(ftok("NOT_DOING_AN", 0), , IPC_CREAT | 0777);
}