#include <iostream>
#include <unistd.h> // 加上这个头文件

#define EXIT_CASE(m) \
    do               \
    {                \
        perror("m"); \
        exit(1);     \
    } while (0)

int main()
{
    pid_t id = fork();
    if (id == 0)
    {
        std::cout << "i am a child process" << std::endl;
    }
    else if (id < 0)
    {
        EXIT_CASE("fork");
    }
    else{
        std::cout<<"i am father process"<<std::endl;
    }
    return 0;
}