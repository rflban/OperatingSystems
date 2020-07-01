#include "myftw.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

int dopath(MyFunc *func, char *filename, int len);

int myftw(char *pathname, MyFunc *func)
{
    return dopath(func, pathname, 0);
}

int dopath(MyFunc *func, char *filename, int len)
{
    struct stat     statbuf;
    struct dirent   *dirp;
    DIR             *dp;
    int             ret;

    if (lstat(filename, &statbuf) < 0)
        return func(filename, &statbuf, FTW_NS, len);

    if (S_ISDIR(statbuf.st_mode) == 0)
        return func(filename, &statbuf, FTW_F, len);

    ret = func(filename, &statbuf, FTW_D, len);

    if (ret != 0)
        return ret;

    dp = opendir(filename);
    if (dp == NULL)
    {
        printf("DEBUG2");
        return func(filename, &statbuf, FTW_DNR, len);
    }

    if (chdir(filename) < 0)
    {
        printf("Cannot chdir into %s\n", filename);
        return func(filename, &statbuf, FTW_DNR, len);
    }

    ++len;

    while ((dirp = readdir(dp)) != NULL)
    {
        if (strcmp(dirp->d_name, ".") != 0 &&
            strcmp(dirp->d_name, "..") != 0)
            dopath(func, dirp->d_name, len);
    }

    if (chdir("..") < 0)
    {
        printf("Cannot return into .. from %s", filename);
        return -1;
    }

    if (closedir(dp) < 0)
    {
        printf("Can't close directory %s\n", filename);
        return -1;
    }

    return ret;
}
