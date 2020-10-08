#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

void errror(char info[], size_t LINE, const char *FILE);

#define ERROR(a) errror(a, __LINE__, __FILE__)
typedef int fd_type;
const pid_t CHILD = 0;

void pipe_safe(int data_pipe[2], int LINE);
pid_t fork_safe(int LINE);
void dup2_safe(int old, int new, int LINE);
void write_safe(int fd, void *buf, size_t size, int LINE);
void read_safe(int fd, void *buf, size_t size, int LINE);

fd_type open_safe(const char *file, int O_FLAG, int LINE);
char *mmap_safe(fd_type file, int LINE);
void close_all(fd_type file, char *map);

void skip_spaces(char **str);

#define pipe_s(a) pipe_safe(a, __LINE__)
#define fork_s() fork_safe(__LINE__)
#define open_s(a, b) open_safe(a, b, __LINE__)
#define mmap_s(a) mmap_safe(a, __LINE__)
#define write_s(a, b, c) write_safe(a, b, c, __LINE__);
#define read_s(a, b, c) read_safe(a, b, c, __LINE__);
#define dup2_s(a, b) dup2_safe(a, b, __LINE__)

//
//
//
//
//
//
//
//
void do_command(char *command);

void do_ordinary_command(char *command, fd_type input, fd_type output);
void do_grep_command(char *command, char *grep);

void print_command_to_pipe(int fd_pipe, const char *main_file);
//
//
int main(int argc, char *argv[])
{
    if (argc != 2)
        ERROR("Not apropriate data for this task!");

    int data_pipe[2] = {};
    pipe_s(data_pipe);
    pid_t main_fork = fork_s();

    if (main_fork != CHILD)
    {
        print_command_to_pipe(data_pipe[1], argv[1]);
        return 0;
    }

    //!THIS IS CHILD
    while (1)
    {
        int count_symbols = 0;
        read_s(data_pipe[0], &count_symbols, sizeof(int));
        if (count_symbols == 0)
        {
            return 0;
        }

        char *command = (char *)calloc(count_symbols, sizeof(char));
        read_s(data_pipe[0], command, count_symbols);

        pid_t new_child = fork_s();
        if (new_child != CHILD)
        {
            do_command(command);
            free(command);
            return 0;
        }
    }
    return 0;
}
//
//
void print_command_to_pipe(int fd_pipe, const char *main_file)
{
    fd_type file = open_s(main_file, O_RDONLY);
    char *file_str = mmap_s(file);

    char *command = NULL;
    char *remaining = file_str;
    while (command = strtok_r(remaining, "\n", &remaining))
    {
        int len = strlen(command);
        write_s(fd_pipe, &len, sizeof(int));
        write_s(fd_pipe, command, len);
    }

    int num = 0;
    write_s(fd_pipe, &num, sizeof(num));
    close_all(file, file_str);
}

void do_command(char *command)
{
    if (!command)
        ERROR("NULL pointer!");

    char *remaining = command;
    char *temp_name = NULL;
    char *last_grep = NULL;
    int count = 0;
    while (temp_name = strtok_r(remaining, "|", &remaining))
    {
        last_grep = temp_name;
        count++;
    }

    if (count > 2)
        ERROR("Now I can't solve this problem - watch this later!)");
    if (count == 1)
        do_ordinary_command(command, STDIN_FILENO, STDOUT_FILENO);

    //!COUNT NOW 2
    do_grep_command(command, last_grep);
}

void do_ordinary_command(char *command, fd_type input, fd_type output)
{
    if (!command)
        ERROR("NULL pointer!");

    if (input != STDIN_FILENO)
        dup2_s(input, STDIN_FILENO);
    if (output != STDOUT_FILENO)
        dup2_s(output, STDOUT_FILENO);

    char *remaining = command;
    int count = 0;
    while (strtok_r(remaining, " ", &remaining))
    {
        count++;
    }

    char **massive = (char **)calloc(count + 1, sizeof(char *));
    remaining = command;
    for (int i = 0; i < count; ++i)
    {
        massive[i] = remaining;
        //printf("%s\n", massive[i]);
        remaining += strlen(remaining) + 1;
    }
    massive[count] = NULL;

    execvp(massive[0], massive);

    free(massive);
}
void do_grep_command(char *command, char *grep)
{
    int data_pipe[2] = {};
    pipe_s(data_pipe);
    pid_t child = fork_s();

    if (child == CHILD)
    {
        skip_spaces(&command);
        do_ordinary_command(command, STDIN_FILENO, data_pipe[1]);
        return;
    }

    child = fork_s();
    if (child == CHILD)
    {
        skip_spaces(&grep);
        do_ordinary_command(grep, data_pipe[0], STDOUT_FILENO);
        return;
    }
    sleep(1);
    kill(child, SIGKILL);
}
//
//
//
//
//
//
//
//
//
//
//
//
//
void skip_spaces(char **str)
{
    if (!str || !*str)
        ERROR("Tried to skip nullptr string!");
    while (**str == ' ')
    {
        (*str)++;
    }
}

void pipe_safe(fd_type data_pipe[2], int LINE)
{
    int out_pipe = pipe(data_pipe);
    if (out_pipe == -1)
        ERROR("PIPE!");
}

pid_t fork_safe(int LINE)
{
    pid_t out = fork();
    if (out == -1)
        ERROR("FORK!");

    return out;
}

fd_type open_safe(const char *file, int O_FLAG, int LINE)
{
    fd_type output = open(file, O_RDONLY);

    struct stat st;
    fstat(output, &st);

    if (output < 0 || !S_ISREG(st.st_mode))
        ERROR("Opening not reg file");

    return output;
}

char *mmap_safe(fd_type file, int LINE)
{
    struct stat st;
    fstat(file, &st);

    void *out = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, file, 0);

    if (out == (void *)(-1))
        ERROR("Mmaping");

    return (char *)out;
}

void close_all(fd_type file, char *map)
{
    struct stat st;
    fstat(file, &st);
    close(file);
    munmap(map, st.st_size);
}

void dup2_safe(int old, int new, int LINE)
{
    int out_check = dup2(old, new);
    if (out_check == -1)
        ERROR("DUP2");
}
void write_safe(int fd, void *buf, size_t size, int LINE)
{
    ssize_t out_check = write(fd, buf, size);
    if (out_check == -1)
        ERROR("WRITE");
}
void read_safe(int fd, void *buf, size_t size, int LINE)
{
    ssize_t out_check = read(fd, buf, size);
    if (out_check == -1)
        ERROR("READ");
}

void errror(char info[], size_t LINE, const char *FILE)
{
    printf("Mistake was found in %Ilu in file: %s", LINE, FILE);
    printf("\n\nProblem: %s\n", info);
    perror("Problem in perror");
    printf("\n");
    assert(0);
}