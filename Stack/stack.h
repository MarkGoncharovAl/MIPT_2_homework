#pragma once
#include "stdlib.h"
#include "../Common_libs/safe_ops.h"

#define UNIQUE_KEY 5493
const char filename[] = "Hah";

#define SIZE stack->owner_[0]
#define SEM stack->owner_[2]

typedef struct
{
    int *owner_;
    void **mem_;
    int cap_;
    key_t key_;
} mystack_t;

void stack_first_init(int key, int size);

/* Attach (create if needed) shared memory stack to the process.
Returns mymystack_t* in case of success. Returns NULL on failure. */
mystack_t *attach_stack(int key, int size);

/* Detaches existing stack from process. 
Operations on detached stack are not permitted since stack pointer becomes invalid. */
int detach_stack(mystack_t *stack);

/* Marks stack to be destroed. Destruction are done after all detaches */
int mark_destruct(mystack_t *stack);

/* Returns stack maximum size. */
int get_size(mystack_t *stack);

/* Returns current stack size. */
int get_count(mystack_t *stack);

/* Push val into stack. */
int push(mystack_t *stack, void *val);

/* Pop val from stack into memory */
int pop(mystack_t *stack, void **val);

void dump(const mystack_t *stack);

void stack_first_init(key_t key, int size)
{
    sem_t sem = semget(key, 1, IPC_CREAT | 0777);
    if (sem == MY_IPC_ERROR)
        ERROR("semget wasn't done properly!");

    int shared_id = shmget(key, size * sizeof(void *) + 3 * sizeof(int), IPC_CREAT | 0777 | IPC_EXCL);
    if (shared_id == MY_IPC_ERROR)
        ERROR("shmget wasn't done properly!");

    int *mem = (int *)shmat(shared_id, NULL, 0);
    if (mem == (int *)MY_IPC_ERROR)
        ERROR("Shmat wasn't done properly!");

    mem[0] = 0;
    mem[1] = size;
    mem[2] = sem;

    sem_increase(sem);

    int out = shmdt((void *)mem);
    if (out == MY_IPC_ERROR)
        ERROR("Shmdt wasn't done properly!");
}

mystack_t *attach_stack(key_t key, int size)
{
    mystack_t *new_stack = (mystack_t *)calloc(1, sizeof(mystack_t));
    if (new_stack == NULL)
    {
        WARNING("Got nullptr after calloc!");
        return new_stack;
    }

    int shared_id = shmget(key, size * sizeof(void *) + 3 * sizeof(int), 0);
    if (shared_id == MY_IPC_ERROR)
        ERROR("shmget wasn't done properly!");

    new_stack->owner_ = (int *)shmat(shared_id, NULL, 0);
    if (new_stack->owner_ == (void *)MY_IPC_ERROR)
        ERROR("Shmat wasn't done properly!");

    new_stack->key_ = key;
    new_stack->cap_ = new_stack->owner_[1];
    new_stack->mem_ = (void **)(new_stack->owner_ + 3);

    return new_stack;
}
int detach_stack(mystack_t *stack)
{
    int out = shmdt((void *)stack->owner_);
    if (out == MY_IPC_ERROR)
        ERROR("Shmdt wasn't done properly!");

    return out;
}
int mark_destruct(mystack_t *stack)
{
    int shared_block_id = shmget(stack->key_, stack->cap_ * sizeof(void *) + 3 * sizeof(int), 0);
    if (shared_block_id == MY_IPC_ERROR)
        ERROR("Shmget wasn't done properly!");

    int out = shmctl(shared_block_id, IPC_RMID, NULL);
    if (out == MY_IPC_ERROR)
        ERROR("shctl wasn't done properly!");

    return out;
}
int get_size(mystack_t *stack)
{
    return SIZE;
}
int get_count(mystack_t *stack)
{
    return stack->cap_;
}
int push(mystack_t *stack, void *val)
{
    if (SIZE >= stack->cap_)
    {
        return -1;
    }

    sem_t cur_sem = SEM;

    sem_decrease(cur_sem);
    stack->mem_[SIZE] = val;
    SIZE++;
    sem_increase(cur_sem);
    return 0;
}
int pop(mystack_t *stack, void **val)
{
    if (SIZE <= 0)
    {
        *val = NULL;
        return -1;
    }

    sem_t cur_sem = SEM;

    sem_decrease(cur_sem);
    SIZE--;
    *val = stack->mem_[SIZE];
    sem_increase(cur_sem);
    return 0;
}

void dump(const mystack_t *stack)
{
    printf("\nDump has started!\n");
    sem_decrease(SEM);
    for (int i = 0; i < SIZE; ++i)
    {
        printf("%d\t", *((int *)(stack->mem_[i])));
    }
    sem_increase(SEM);
    printf("\nDump has finished!\n");
}
