#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>

void childwrite(int fd)
{
    char buff[1024];
    memset(buff, 0, sizeof(buff));
    snprintf(buff, sizeof(buff), "hello world\n");
    size_t n=write(fd, buff, strlen(buff));
    if (n < 0)
    {
        perror("write error");
        exit(1);
    }
}

void fatherread(int fd)
{
    char buff[1024];
    memset(buff, 0, sizeof(buff));
    int n = read(fd, buff, sizeof(buff) - 1);
    if (n < 0)
    {
        perror("read error");
        exit(1);
    }
    buff[n] = 0;
    printf("%s", buff);
}
int main()
{
    int fds[2];
    size_t m = pipe(fds);
    if (m < 0)
    {
        perror("pipe error");
        exit(1);
    }
    int pid = fork();
    if (pid == 0)
    {
        close(fds[0]);
        childwrite(fds[1]);
        close(fds[1]);
        exit(0);//该处缺失会导致子进程执行父进程的代码
    }
    else if (pid<0)
    {
        perror("fork error");
        exit(1);
    }
    
    close(fds[1]);
    fatherread(fds[0]); 
    waitpid(pid, NULL, 0);
    close(fds[0]);
    return 0;
}
