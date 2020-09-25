#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

void errror(char info[], size_t LINE, const char* FILE);
#define ERROR(a) errror(a, __LINE__, __FILE__)


typedef char bool;
const bool true  = 1;
const bool false = 0;

typedef enum {NOT_NUM = 0, NOTHING, THREE, FIVE, FIFTEEN} BIZZ_BUZZ;

//from to
typedef struct {
    int input_;
    int output_;
} fd;


int ok(int fd_read);

fd read_1_file (const char* in);
fd read_2_files(const char* in, const char* out);

void copy_files(fd files);
void safe_write(int file, const char* ptr, __off_t bytes);

void bizz_buzz (fd files);
void write_bizz(BIZZ_BUZZ num, int output_file, const char* pt_num);

BIZZ_BUZZ read_num(char** str, size_t* remaining);
BIZZ_BUZZ compare_dels(int del3, int del5);

void close_files(fd files);

int main(int argc, char** argv)
{
    fd files;

    if (argc == 2)
        files = read_1_file(argv[1]);
    else if (argc == 3)
        files = read_2_files(argv[1], argv[2]);
    else {
        //argc != 2 != 3
        ERROR("The input format isn't wrong!");
    }

    //copy_files (files);
    bizz_buzz  (files);
    close_files(files);
    return 0;
}

fd read_1_file(const char* in)
{
    fd result;
    result.input_ = open(in, O_RDONLY);

    char name_out[] = "LOOK_AT_ME";
    result.output_ = open(name_out, O_CREAT | O_WRONLY | O_TRUNC, 0777);

    if (result.input_ < 0 || result.output_ < 0 || !ok(result.input_))
        ERROR("File wasn't opened or created properly!");

    return result;
}
fd read_2_files(const char* in, const char* out)
{
    fd result;
    result.input_ = open(in, O_RDONLY);

    result.output_ = open(out, O_CREAT | O_WRONLY | O_TRUNC, 0777);
    if (result.input_ < 0 || result.output_ < 0 || !ok(result.input_))
        ERROR("File wasn't opened or created properly!");

    return result;
}

void copy_files(fd files)
{
    struct stat stat_input;
    fstat(files.input_, &stat_input);

    void* ptr_input = mmap(NULL, stat_input.st_size, PROT_READ, MAP_PRIVATE, files.input_, 0);
    safe_write(files.output_, ptr_input, stat_input.st_size);
    munmap(ptr_input, stat_input.st_size);
}

//BIIZ_BUZZ
//------------------------------------------------------
void bizz_buzz(fd files)
{
    struct stat stat_input;
    fstat(files.input_, &stat_input);

    char* ptr_input = (char*)mmap(NULL, stat_input.st_size, PROT_READ, MAP_PRIVATE, files.input_, 0);

    char** str = &ptr_input;
    const char* previous_num = *str;

    BIZZ_BUZZ cur_num = NOT_NUM;
    size_t remaining = stat_input.st_size - 1;

    while ((cur_num = read_num(str, &remaining)) != NOT_NUM) {
        write_bizz(cur_num, files.output_, previous_num);
        previous_num = *str;
    }

    munmap(ptr_input, stat_input.st_size);
}
void write_bizz(BIZZ_BUZZ num, int output, const char* pt_num)
{
    if (num == FIFTEEN) {
        char out[] = "bizzbuzz ";
        safe_write(output, out, sizeof(out));
        return;
    }
    if (num == THREE) {
        char out[] = "bizz ";
        safe_write(output, out, sizeof(out));
        return;
    }
    if (num == FIVE) {
        char out[] = "buzz ";
        safe_write(output, out, sizeof(out));
        return;
    }
    //Now not a bizz-buzz and so on...
    while(*pt_num == ' ' || *pt_num == '\n')
        pt_num++;

    if (*pt_num == '-') {
        safe_write(output, pt_num, 1);
        pt_num++;
    }

    while (*pt_num >= '0' && *pt_num <= '9') {
        safe_write(output, pt_num, 1);
        pt_num++;
    }

    char buf[] = " ";
    safe_write(output, buf, sizeof(buf));
}
BIZZ_BUZZ read_num(char** str, size_t* remaining)
{
    while (**str == ' ') {
        (*str)++;
        (*remaining)--;
    }

    char cur_symbol = **str;
    if (cur_symbol == '\0' || cur_symbol == EOF || *remaining <= 0)
        return NOT_NUM;

    //now everything is ok
    if (cur_symbol == '-') {
        (*str)++;
        (*remaining)--;
        cur_symbol = **str;
    }

    int del3 = 0, del5 = 0;
    while(cur_symbol != ' ' && cur_symbol != '\0' && cur_symbol != EOF && *remaining > 0) {

        //printf("%c", cur_symbol);
        if (cur_symbol < '0' || cur_symbol > '9') {
            ERROR("Not a number found!");
        }

        del3 = (del3 + (cur_symbol - '0')) % 3;
        del5 = (cur_symbol - '0') % 5;

        (*str)++;
        (*remaining)--;
        cur_symbol = **str;
    }

    return compare_dels(del3, del5);
}
BIZZ_BUZZ compare_dels(int del3, int del5)
{
    if (del3 == 0) {
        if (del5 == 0)
            return FIFTEEN;
        return THREE;
    }
    if (del5 == 0)
        return FIVE;
    return NOTHING;
}
//------------------------------------------------------

void close_files(fd files)
{
    close(files.input_);
    close(files.output_);
}


//CHECKING FUNCTIONS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++
int ok(int fd_read)
{
    struct stat stat_input;
    fstat(fd_read, &stat_input);

    return (S_ISREG(stat_input.st_mode));
}
void errror(char info[], size_t LINE, const char* FILE)
{
    printf("Mistake was found in %Ilu in file: %s", LINE, FILE);
    printf("\n\nProblem: %s\n\n", info);
    assert(0);
}
void safe_write(int file, const char* ptr, __off_t bytes)
{
    int out_number = write(file, ptr, bytes);
    if (out_number == -1)
        ERROR("Data wasn't written in file properly!");
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++
