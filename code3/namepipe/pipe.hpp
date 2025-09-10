#pragma once

#include <iostream>
#include <cstdio>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define PATH "."
#define EXIT_CASE(m) \
    do               \
    {                \
        perror("m"); \
        exit(1);     \
    } while (0)
using namespace std;

class Pipe
{
public:
    Pipe(const string &_path, const string &name)
        : path(_path),
          name(name)
    {
        fifo = _path + '/' + name;
    }
    void Creat()
    {
        umask(0);
        int n = mkfifo(fifo.c_str(), 0777);
        if (n < 0)
        {
            EXIT_CASE("mkfifo");
        }
        cout << "creat success" << endl;
    }
    void ROpen()
    {
        fd = open(fifo.c_str(), O_RDONLY);
        if (fd < 0)
        {
            EXIT_CASE("open");
        }
        cout << "read open success" << endl;
    }
    void Wopen()
    {
        fd = open(fifo.c_str(), O_WRONLY | O_TRUNC);
        if (fd < 0)
        {
            EXIT_CASE("m");
        }
        cout << "write open sucess" << endl;
    }
    void Write()
    {
        while (1)
        {
            string buff;
            cout << "Please input for your keyboard" << endl;
            getline(cin, buff);
            cout << buff << endl;
            write(fd, buff.c_str(), buff.size());
        }
    }
    void Read()
    {
        while (1)
        {
            char buffer[1024];
            int i = read(fd, buffer, sizeof(buffer) - 1);
            if (i < 0)
            {
                EXIT_CASE("read");
                break;
            }
            else if (i == 0)
            {
                cout << "write exit me too" << endl;
                break;
            }
            else
            {
                buffer[i] = 0;
                cout << "read success" << buffer << endl;
            }
        }
    }
    void destroy()
    {
        if (fd > 0)
        {
            close(fd);
        }
    }
    ~Pipe()
    {
    }

private:
    string path;
    string name;
    string fifo;
    int fd;
};