#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

struct stat stat_buffer;

int main(int argc , char **argv)
{
    if(argc != 2)
    {
        perror("Eroare argumente");
        exit(0);
    }
    if(lstat(argv[1] , &stat_buffer)!=0)
    {
        perror("Eroare director");
        exit(0);
    }
    if(S_ISDIR(stat_buffer.st_mode))
    {
        printf("este director");
    }
    return 0;
}