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
#include <ctype.h>
#include <sys/wait.h>
#define SIZE 1024

int count_corruped=0;
char array_distinct[10][100]={""};
int size_array=0;
struct stat stat_buffer;

void clonare_snaphot(char sn1[], int fd1, char sn2[], int fd2)
{
    lseek(fd1, 0, SEEK_SET);
    lseek(fd2, 0, SEEK_SET);
    char buf2[SIZE];
    int count1 = 0;
    while ((count1 = read(fd2, buf2, SIZE)) > 0)
    {
        write(fd1, buf2, count1);
    }
}

int comparare_snapshot(char sn1[], int fd1, char sn2[], int fd2)
{
    fstat(fd1, &stat_buffer);
    int size1 = stat_buffer.st_size;
    fstat(fd2, &stat_buffer);
    int size2 = stat_buffer.st_size;
    if (size1 != size2)
    {
        return 1;
    }
    else
    {
        char buf1[SIZE] = "\0";
        char buf2[SIZE] = "\0";
        int count1;
        int count2;
        lseek(fd1, 0, SEEK_SET);
        lseek(fd2, 0, SEEK_SET);
        while ((count1 = read(fd1, buf1, SIZE)) > 0 && (count2 = read(fd2, buf2, SIZE)) > 0)
        {
            if (strlen(buf1) != strlen(buf2))
            {
                return 1;
            }
            for (int i = 0; i < strlen(buf1); i++)
            {
                if (buf1[i] != buf2[i])
                {
                    return 1;
                }
            }
        }
    }
    return 0;
}

void salvare_snaphot(int fd, char *cale_director)
{
    char aux[SIZE] = "";

    int length_buf = snprintf(aux, SIZE, "%s %ld %s\n", cale_director, stat_buffer.st_size, ctime(&stat_buffer.st_mtime));
    write(fd, aux, length_buf);
}

void creare_snaphot(int fd, char *cale_director , char izolare[])
{
    int pid, wstatus;
    pid_t w;
    DIR *director = opendir(cale_director);
    if (director == NULL)
    {
        perror("Eroare director");
        return;
    }
    struct dirent *dir_curent;
    while ((dir_curent = readdir(director)) != NULL)
    {
        if (strcmp(dir_curent->d_name, ".") != 0 && strcmp(dir_curent->d_name, "..") != 0)
        {
            char array_cale[SIZE];
            snprintf(array_cale, sizeof(array_cale), "%s/%s", cale_director, dir_curent->d_name);
            if (lstat(array_cale, &stat_buffer) == 0)
            {
                if (S_ISREG(stat_buffer.st_mode))
                {
                    if ((stat_buffer.st_mode & (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH)) == 0)
                    {
                        int pipefd[2];

                        if(pipe(pipefd)<0)
                        {
                            perror("eroare pipe");
                            exit(0);
                        }
                        if ((pid=fork())<0)
                        {
                            perror("eroare creare proces");
                            exit(0);
                        }
                        if (pid==0)
                        {
                            close(pipefd[0]);
                            dup2(pipefd[1] , 1);
                            execlp("./script.sh", "script.sh", array_cale , NULL);
                            perror("eroare script");
                            exit(0);
                        }
                        else{
                            close(pipefd[1]);
                            char message[100]="";
                            int count=read(pipefd[0] , message , sizeof(message));
                            close(pipefd[0]);
                            if(count<0)
                            {
                                perror("eroare citire pipe");
                                exit(0);
                            }
                            message[strcspn(message , "\n")]='\0';
                            if(strcmp(message , "SAFE")!=0)
                            {
                                count_corruped++;
                                char nume_fis[150]="";
                                char cale_noua[300]="";
                                char *p=strrchr(array_cale , '/');
                                if(p)                                    
                                {
                                    strcpy(nume_fis , p+1);
                                }
                                snprintf(cale_noua , sizeof(cale_noua) , "%s/%s" , izolare , nume_fis);
                                rename(array_cale , cale_noua);
                            }
                            w=wstatus = wait(&wstatus);
                            if(w==-1)
                            {
                                perror("eroare wait");
                                exit(0);
                            }
                        }
                    }
                    else salvare_snaphot(fd, array_cale);
                }
                else if(S_ISDIR(stat_buffer.st_mode) && S_ISLNK(stat_buffer.st_mode)==0)
                {
                    creare_snaphot(fd, array_cale , izolare);
                }
            }
        }
    }
    closedir(director);
}

int main(int argc, char **argv)
{
    pid_t w;
    int wstatus , pid;
    if (argc > 12)
    {
        perror("Eroare argumente");
        exit(0);
    }

    for(int i=5 ; i<argc ; i++)
    {
        if (lstat(argv[i], &stat_buffer)!=0)
        {
            perror("Eroare director");
            continue;
        }
        if (S_ISDIR(stat_buffer.st_mode) && S_ISLNK(stat_buffer.st_mode)==0)
        {
            int valid=0;
            for(int j=0 ; j<size_array ; j++)
            {
                if(strcmp(array_distinct[j] , argv[i])==0)
                {
                    valid=1;
                    break;
                }
            }
            if(valid==0)
            {
                strcpy(array_distinct[size_array++] , argv[i]);
            }
        }
    }

    char dir_out[SIZE];
    strcpy(dir_out, argv[2]);

    // do
    // {
        for (int i = 0; i < size_array; i++)
        {
            if (lstat(array_distinct[i], &stat_buffer) != 0)
            {
                perror("Eroare director");
                continue;
            }
            if (S_ISDIR(stat_buffer.st_mode) == 0)
            {
                perror("Nu este director");
                continue;
            }
            if ((pid = fork()) < 0)
            {
                perror("eroare creare proces");
                exit(0);
            }
            if (pid == 0)
            {
                count_corruped=0;
                char nr1[10] = "";
                int ino1 = stat_buffer.st_ino;
                sprintf(nr1, "%d", ino1);
                char nume_dir1[SIZE] = "";
                strcat(nume_dir1, dir_out);
                strcat(nume_dir1, "/snapshot_");
                strcat(nume_dir1, nr1);
                strcat(nume_dir1, "_1.txt");

                int fd1 = open(nume_dir1, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH);
                if (fd1 == -1)
                {
                    perror("Eroare fisier");
                    exit(-1);
                }
                if (lstat(nume_dir1, &stat_buffer) == 0)
                {
                    if (stat_buffer.st_size == 0)
                    {
                        creare_snaphot(fd1, array_distinct[i] , argv[4]);
                    }
                    char nr2[10] = "";
                    int ino2 = stat_buffer.st_ino;
                    sprintf(nr2, "%d", ino2);
                    char nume_dir2[SIZE] = "";
                    strcat(nume_dir2, dir_out);
                    strcat(nume_dir2, "/snapshot_");
                    strcat(nume_dir2, nr2);
                    strcat(nume_dir2, "_2.txt");

                    int fd2 = open(nume_dir2, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH);
                    if (fd2 == -1)
                    {
                        perror("Eroare fisier");
                        exit(-1);
                    }

                    creare_snaphot(fd2, array_distinct[i] , argv[4]);
                    if (comparare_snapshot(nume_dir1, fd1, nume_dir2, fd2) == 1)
                    {
                        clonare_snaphot(nume_dir1, fd1, nume_dir2, fd2);
                        unlink(nume_dir2);
                    }
                    else
                    {
                        close(fd2);
                        unlink(nume_dir2);
                    }
                }
                exit(count_corruped);
            }
        }
        for(int i=0 ; i<size_array ; i++)
        {
            w = wait(&wstatus);
            if(w==-1)
            {
                perror("eroare wait");
                exit(0);
            }
            if (WIFEXITED(wstatus)) {
                printf("Procesul %d cu PID=%d s-a terminat cu numarul de fisiere corupte returnate %d\n", i , w , WEXITSTATUS(wstatus));
            } else if (WIFSIGNALED(wstatus)) {
                printf("killed by signal %d\n", WTERMSIG(wstatus));
            } else if (WIFSTOPPED(wstatus)) {
                printf("stopped by signal %d\n", WSTOPSIG(wstatus));
            } else if (WIFCONTINUED(wstatus)) {
                printf("continued\n");
            }
        }
    return 0;
}