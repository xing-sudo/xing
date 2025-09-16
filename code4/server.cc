#include "shm.hpp"
#include <unistd.h>
int main()
{
    Shm test(".", 0x666, "server");
    int x = test.KeyGet();
    printf("my key is %x", x);
    char *men = (char *)test.VtaddGet();//使用类似于malloc
    int count = 0;
    while (true)
    {
        sleep(2);
        printf("%s\n", men);
        if (men[count] == 'z')
        {
            count++;
        }
        else
        {
            break;
        }
    }

    return 0;
}
