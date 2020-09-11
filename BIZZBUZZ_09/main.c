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

#define bool  char
#define true  1
#define false 0

//from to
typedef struct {
    int input_;
    int output_;
} fd;
typedef enum {NOT_NUM = 0, NOTHING, THREE, FIVE, FIFTEEN} BIZZ_BUZZ;


int ok(int fd_read);

fd read_1_file (const char* in);
fd read_2_files(const char* in, const char* out);

void copy_files(fd files);


void bizz_buzz (fd files);
void write_bizz(BIZZ_BUZZ num, int output_file);
//returns true == 1 if num was read successfully
BIZZ_BUZZ read_num(char** str, int* remaining);
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
        printf("The input format isn't wrong!");
        assert(0);
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
    assert(ok(result.input_));

    char name_out[] = "LOOK_AT_ME";
    result.output_ = open(name_out, O_CREAT | O_WRONLY, 0777);

    assert(result.input_ >= 0 && result.output_ >= 0);
    return result;
}
fd read_2_files(const char* in, const char* out)
{
    fd result;
    result.input_ = open(in, O_RDONLY);
    assert(ok(result.input_));

    result.output_ = open(out, O_CREAT | O_WRONLY | O_TRUNC, 0777);
    assert(result.input_ >= 0 && result.output_ >= 0);
    return result;
}

void copy_files(fd files)
{
    struct stat stat_input;
    fstat(files.input_, &stat_input);

    void* ptr_input = mmap(NULL, stat_input.st_size, PROT_READ, MAP_PRIVATE, files.input_, 0);
    write(files.output_, ptr_input, stat_input.st_size);
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
    BIZZ_BUZZ cur_num = NOT_NUM;
    int remaining = stat_input.st_size - 1;

    while ((cur_num = read_num(str, &remaining)) != NOT_NUM) {
        write_bizz(cur_num, files.output_);
    }

    munmap(ptr_input, stat_input.st_size);
}
void write_bizz(BIZZ_BUZZ num, int output)
{
    if (num == FIFTEEN) {
        char out[] = "bizzbuzz ";
        write(output, out, sizeof(out));
        return;
    }
    if (num == THREE) {
        char out[] = "bizz ";
        write(output, out, sizeof(out));
        return;
    }
    if (num == FIVE) {
        char out[] = "buzz ";
        write(output, out, sizeof(out));
    }
}
BIZZ_BUZZ read_num(char** str, int* remaining)
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
//++++++++++++++++++++++++++++++++++++++++++++++++++++++