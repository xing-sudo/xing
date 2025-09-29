#include <iostream>
#include <sys/types.h>
#include <signal.h>


int main(int agrc, char *agrv[])
{
    if (agrc != 3)
    {
        std::cout << "./mykill -num -pid" << std::endl;
        return 0;
    }
    int nums = std::stoi(agrv[1]);
    pid_t id = std::stoi(agrv[2]);
    int n = kill(nums, id);
    if (n == 0)
    {
        std::cout << "send" << nums << "to" << id << "sucess" << std::endl;
    }
}