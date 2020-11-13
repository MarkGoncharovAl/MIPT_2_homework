#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>
#include "stack.h"

#define GET_KEY(var)                 \
    key_t var = ftok("Hah", rand()); \
    if (var == MY_IPC_ERROR)         \
    ERROR("ftok wasn't done properly!")

void test1();
void test2();
void test3();

int main(int argc, char *argv[])
{
    srand(time(0));

    test1();
    printf("\n\nTESTED -- 1!!!\n\n");
    test2();
    printf("\n\nTESTED -- 2!!!\n\n");
    test3();
    printf("\n\nTESTED -- 3!!!\n\n");
}

void test1()
{

    GET_KEY(key);
    GET_KEY(extra_key1);
    GET_KEY(extra_key);

    sem_t main_sem = semget(extra_key1, 1, IPC_CREAT | 0777);
    sem_t extra_sem = semget(extra_key, 1, IPC_CREAT | 0777);

    if (main_sem == -1 || extra_sem == -1)
    {
        ERROR("SEMAPHORS");
    }

    pid_t pd = fork();

    if (pd == 0)
    {
        stack_first_init(key, 21);

        mystack_t *my_st = attach_stack(key, 21);

        int data[] = {6, 7, 8, 9, 10};
        for (int i = 0; i < 5; ++i)
        {
            push(my_st, (void *)(data + i));
            dump(my_st);
        }

        sem_increase(main_sem);
        sem_decrease(extra_sem);
        dump(my_st);
        detach_stack(my_st);

        free(my_st);

        raise(SIGKILL);
    }

    sem_decrease(main_sem);

    mystack_t *my_st = attach_stack(key, 21);

    int data[] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; ++i)
    {
        push(my_st, (void *)(data + i));
        dump(my_st);
    }

    detach_stack(my_st);

    sem_increase(extra_sem);
    struct timespec time = {0, 10000000};
    nanosleep(&time, NULL);
    mark_destruct(my_st);
    free(my_st);
}

void test2()
{
    //одновременно push с разных мест
    GET_KEY(key_sem);
    GET_KEY(key_stack);

    sem_t sem = semget(key_sem, 1, IPC_CREAT | 0777);
    if (sem == MY_IPC_ERROR)
        ERROR("sem wasn't created properly!");

    pid_t pd = fork_s();
    if (pd == 0)
    {
        int data[] = {1, 2, 3, 4, 5};
        sem_decrease(sem);
        mystack_t *my_st = attach_stack(key_stack, 10);

        for (int i = 0; i < 5; ++i)
        {
            push(my_st, data + i);
            dump(my_st);
        }

        for (int i = 0; i < 5; ++i)
        {
            void *val = NULL;
            pop(my_st, &val);
            printf("Popped kids value : %d\n", *(int *)val);
            dump(my_st);
        }

        detach_stack(my_st);
        sem_increase(sem);

        free(my_st);
        raise(SIGKILL);
    }

    stack_first_init(key_stack, 10);
    int data[] = {6, 7, 8, 9, 10};
    sem_increase(sem);
    mystack_t *my_st = attach_stack(key_stack, 10);

    for (int i = 0; i < 5; ++i)
    {
        push(my_st, data + i);
        dump(my_st);
    }

    for (int i = 0; i < 5; ++i)
    {
        void *val = NULL;
        pop(my_st, &val);
        printf("Popped main value : %d\n", *(int *)val);
        dump(my_st);
    }

    detach_stack(my_st);
    sem_decrease(sem);

    mark_destruct(my_st);
    free(my_st);
}

void test3()
{
    GET_KEY(key);
    stack_first_init(key, 2);
    mystack_t *my_st = attach_stack(key, 2);

    int data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    for (int i = 0; i < 10; ++i)
    {
        int out = push(my_st, data + i);
        dump(my_st);
        if (i < 2 && out == -1 ||
            i >= 2 && out != -1)
        {
            ERROR("Push out of range is dead!");
        }
    }

    for (int i = 0; i < 10; ++i)
    {
        void *val = NULL;
        int out = pop(my_st, &val);
        dump(my_st);
        if (i < 2 && out == -1 ||
            i >= 2 && out != -1)
        {
            ERROR("Pop out of range is dead!");
        }
    }

    detach_stack(my_st);
    mark_destruct(my_st);

    free(my_st);
}
