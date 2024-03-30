#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#define SIZE 1000

struct stat stat_buffer;

void salvare_snaphot(int fd , char *cale_director)
{
    if(lstat(cale_director , &stat_buffer)==0)
    {
        if(S_ISREG(stat_buffer.st_mode))
        {
            dprintf(fd , "%s %ld %s\n" , cale_director , stat_buffer.st_size , ctime(&stat_buffer.st_mtime));
        }
    }
}

void creare_snaphot(int fd , char *cale_director)
{
    DIR *director=opendir(cale_director);
    if(director==NULL)
    {
        perror("Eroare director");
        return;
    }
    struct dirent *dir_curent;
    while((dir_curent=readdir(director))!=NULL)
    {
        if(strcmp(dir_curent->d_name , ".")!=0 && strcmp(dir_curent->d_name , "..")!=0)
        {
            char array_cale[SIZE];
            snprintf(array_cale , sizeof(array_cale) , "%s/%s" , cale_director , dir_curent->d_name);
            if(lstat(array_cale , &stat_buffer)==0)
            {
                if(S_ISREG(stat_buffer.st_mode))
                {
                    salvare_snaphot(fd , array_cale);
                }
                else if(S_ISDIR(stat_buffer.st_mode))
                {
                    creare_snaphot(fd , array_cale);
                }
            }
        }
    }
    closedir(director);
}

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
    if(S_ISDIR(stat_buffer.st_mode)==0)
    {
        perror("Nu este director");
        exit(0);
    }
    int fd=open("snapshot.txt" , O_RDWR | O_CREAT , S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH);
    if(fd==-1)
    {
        perror("Eroare fisier");
        exit(-1);
    }

    creare_snaphot(fd , argv[1]);

    close(fd);
    return 0;
}