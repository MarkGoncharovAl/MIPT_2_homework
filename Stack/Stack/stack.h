#pragma once
#include "stdlib.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include "errno.h"

typedef int sem_t;
typedef char bool_t;

#define UNIQUE_KEY 5493

#define SIZE stack->owner_[0]
#define SEM stack->owner_[2]

#define MY_IPC_ERROR -1
#define TRUE 1
#define FALSE 0
#define filename "Hah"

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

//MY_FUNCTIONS
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
void dump(const mystack_t *stack, const char extra_info[]);
void errror(const char info[], size_t LINE, char *FILE);
void warrning(const char info[], size_t LINE, char *FILE);
#define ERROR(a) errror(a, __LINE__, __FILE__)
#define WARNING(a) warrning(a, __LINE__, __FILE__)

pid_t fork_s();

void sem_increase(sem_t sem);
void sem_decrease(sem_t sem);
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////