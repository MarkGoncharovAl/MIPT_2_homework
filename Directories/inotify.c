#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>

typedef int fd_t;

int main(int argc, char *argv[])
{
    /*
    DIR *opendir(const char *name);
    DIR *fdopendir(int fd);
    readdir 3
    DT_
    rewinddir
    seekdir
    scandir - DON'T
    telldir
    closedir
    inotify

    */
    /*
    for (int i = 1; i < argc; ++i)
    {
        DIR *dir = opendir(argv[i]);
        if (dir == NULL)
        {
            printf("File %s can't be opened!\n", argv[i]);
            continue;
        }

        struct dirent *out;
        while ((out = readdir(dir)) != NULL)
        {
            printf("%s\n", out->d_name);
        }

        closedir(dir);
    }
    */
    if (argc != 2)
    {
        perror("Not enough amount of parameters!");
        return 0;
    }

    fd_t in = inotify_init();
    if (in == -1)
    {
        perror("Init is bad!\n");
        return -1;
    }

    uint32_t mask = IN_CREATE | IN_DELETE | IN_MOVE_SELF;
    int watch = inotify_add_watch(in, argv[1], mask);

    if (watch == -1)
    {
        perror("Watch is bad!\n");
        return -1;
    }

    char buf[4096] = {};
    while (1)
    {
        int check_read = read(in, buf, 4096);
        if (check_read > 0)
        {
            struct inotify_event *event = (struct inotify_event *)buf;
            printf("Path: %s ", event->name);
            if (event->mask & IN_CREATE)
                printf("Event is creating!\n");
            if (event->mask & IN_DELETE)
                printf("Event is deleting!\n");
            if (event->mask & IN_MOVE_SELF)
                printf("Event is moving now!\n");
        }
        else
        {
            perror("Reading was unsuccessful!\n");
            return -1;
        }
    }

    close(in);
    return 0;
}

// !For next homework
//2 fork()
//man 2 setsid, setgit
// parent = init
// man 7 daemon