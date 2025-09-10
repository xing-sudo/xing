#include"pipe.hpp"

int main()
{
    Pipe p1(".","fifo");
    p1.Creat();
    p1.ROpen();
    p1.Read();
    p1.destroy();
    return 0;
}