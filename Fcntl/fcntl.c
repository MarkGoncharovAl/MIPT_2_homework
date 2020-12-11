#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <dirent.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <sys/file.h>

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Not enough arguments!");
        return 0;
    }

    int fd = open(argv[1], O_RDWR);

    if (fd < 0)
    {
        perror("Can't open file properly!");
        return 1;
    }

    // !TESTS
    ///////////////////////////////////////////////////////////
    /* struct flock f1 = {0};
     f1.l_type = F_WRLCK;
     f1.l_whence = SEEK_SET;
     f1.l_len = 0; //full file
     f1.l_pid = 0;
     int ret = fcntl(fd, F_SETFL, &f1);
     if (ret < 0)
     {
         perror("Lock wasn't done properly!\n");
         return 1;
     }*/
    ///////////////////////////////////////////////////////////

    struct flock f_read = {0};
    while (fcntl(fd, F_GETLK, &f_read) >= 0)
    {
        printf("Checking locks:\n");
        printf("Type: %d\nWhence: %d\nLen: %ld\nPid: %d\nStart: %ld\n\n", f_read.l_type, f_read.l_whence, f_read.l_len, f_read.l_pid, f_read.l_start);
    }

    close(fd);
    return 0;
}
