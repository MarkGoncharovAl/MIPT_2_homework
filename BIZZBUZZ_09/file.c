#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>

void errror(char info[], size_t LINE, const char* FILE);
#define ERROR(a) errror(a, __LINE__, __FILE__)


typedef char bool;
const bool true  = 1;
const bool false = 0;

typedef enum {END = 0, LETTERS, NOTHING, THREE, FIVE, FIFTEEN} BIZZ_BUZZ;

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
void write_bizz(BIZZ_BUZZ num, int output_file);

BIZZ_BUZZ read_num(const char* str);
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
        ERROR("The input format isn't right!");
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

    char* ptr_input = (char*)mmap(NULL, stat_input.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, files.input_, 0);

    const char* str_strtok = strtok(ptr_input, " ");
    const char* prev_str = str_strtok;

    while (str_strtok) {

        if (str_strtok > prev_str) {
            safe_write(files.output_, prev_str, str_strtok - prev_str);
            safe_write(STDOUT_FILENO, prev_str, str_strtok - prev_str);
        }
        prev_str = str_strtok + sizeof(str_strtok) + 1;

        BIZZ_BUZZ num = read_num(str_strtok);
        switch (num)
        {
        case LETTERS:
            safe_write(files.output_, str_strtok, strlen(str_strtok));
            break;
        case END :
            ERROR("Something went really strange...");
            break;
        case NOTHING:
            safe_write(files.output_, str_strtok, strlen(str_strtok));
            break;
        default:
            write_bizz(num, files.output_);
            break;
        }

        str_strtok = strtok(NULL, " ");
        
        if (str_strtok)
            safe_write(files.output_, " ", 1);
    }

    munmap(ptr_input, stat_input.st_size);
}
void write_bizz(BIZZ_BUZZ num, int output)
{
    const char out1[] = "bizzbuzz";
    const char out2[] = "bizz";
    const char out3[] = "buzz";

    switch (num)
    {
    case FIFTEEN:
        safe_write(output, out1, sizeof(out1));
        break;
    case THREE:
        safe_write(output, out2, sizeof(out2));
        break;
    case FIVE:
        safe_write(output, out3, sizeof(out3));
        break;
    default:
        ERROR("Something went wrong...");
        break;
    }
}
BIZZ_BUZZ read_num(const char* str)
{
    char cur_symbol = *str;
    if (cur_symbol == '\0' || cur_symbol == EOF)
        return END;

    //now everything is ok
    if (cur_symbol == '-') {
        str++;
        cur_symbol = *str;
    }

    int del3 = 0, del5 = 0;
    while(cur_symbol >= '0' && cur_symbol <= '9' && cur_symbol != '\0' && cur_symbol != EOF && cur_symbol != '\n') {

        del3 = (del3 + (cur_symbol - '0')) % 3;
        del5 = (cur_symbol - '0') % 5;
        str++;
        cur_symbol = *str;
    }
    if (cur_symbol != '\0' && cur_symbol != EOF && cur_symbol != '\n')
        return LETTERS;

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
