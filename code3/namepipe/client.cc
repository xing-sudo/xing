#include"pipe.hpp"

int main()
{
    Pipe p2(".","fifo");
    p2.Wopen();
    p2.Write();
    p2.destroy();
    return 0;
}

