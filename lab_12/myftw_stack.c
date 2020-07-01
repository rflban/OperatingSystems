#include "myftw.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "stack.h"

int dopath(MyFunc *func, char *filename);

int myftw(char *pathname, MyFunc *func)
{
    return dopath(func, pathname);
}

int dopath(MyFunc *func, char *filename)
{
    struct stat     statbuf;
    struct dirent   *dirp;
    DIR             *dp;
    int             len = 0;

    struct stack *stack = stack_create();
    stack_push(stack, filename);

    while (!stack_is_empty(stack))
    {
        const char *filename = stack_pop(stack);

        if (strcmp(filename, "..") == 0)
        {
            --len;

            if (chdir(filename) < 0)
            {
                printf("Cannot return into .. from");
                return -1;
            }
        }
        else if (lstat(filename, &statbuf) < 0)
            func(filename, &statbuf, FTW_NS, len);
        else if (S_ISDIR(statbuf.st_mode) == 0)
            func(filename, &statbuf, FTW_F, len);
        else
        {
            func(filename, &statbuf, FTW_D, len);

            dp = opendir(filename);

            if (dp == NULL)
                func(filename, &statbuf, FTW_DNR, len);

            if (chdir(filename) < 0)
            {
                printf("Cannot chdir to %s\n", filename);
                func(filename, &statbuf, FTW_DNR, len);
            }
            else
            {
                ++len;

                stack_push(stack, "..");

                while ((dirp = readdir(dp)) != NULL)
                {
                    if (strcmp(dirp->d_name, ".")  != 0 &&
                        strcmp(dirp->d_name, "..") != 0)
                        stack_push(stack, dirp->d_name);
                }
            }
        }
    }

    return 0;
}
