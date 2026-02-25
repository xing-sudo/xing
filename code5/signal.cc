#include <iostream>
#include <signal.h>
void handle_1(int x)
{
    //alarm(5);//外面设置闹钟倒计时结束
    int cnt =5;
    while (cnt--)
    {
        //alarm(5);///放里面重复设置闹钟
        sleep(1);
        std::cout << "get a signal num is " << x << std::endl;
    }
}

void handle_2(int x)
{
    std::cout<<"信号"<<x<<"递达!"<<std::endl;
    sigset_t pending;
    sigpending(&pending);
    for (int i = 1; i <=31; i++)
    {
        if(sigismember(&pending,i))
        {
            std::cout<<"1";
        }
        else{
            std::cout<<"0";
        }
    }
    std::cout<<std::endl;
}

int main()
{
    signal(2, handle_1);
    //abort();//只对自己的自定义进行恢复默认
    // while (true)
    // {
    //     sleep(1);
    //     std::cout << "hello world" << std::endl;
    // }
    pause();//与os的本质相同一个死循环处理不同的中断，执行不同的中断方法
    return 0;
}