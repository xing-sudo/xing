#include"shm.hpp"
#include<unistd.h>
int main()
{
    Shm client(".",0x666,"user");
    char*c=(char*)client.VtaddGet();
    for (char i = 'a'; i <= 'z'; i++)
    {
        sleep(1);
        c[i-'a']=i;
    }
    
}