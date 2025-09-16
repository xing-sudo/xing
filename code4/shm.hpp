#pragma once
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string>
#include <iostream>
#include <cstdio>

#define EXIT_CASE(m) \
    do               \
    {                \
        perror("m"); \
        exit(1);     \
    } while (0)
const int gmode = 0666;
const std::string path = ".";
const int projid = 0x666;
const int size = 4096;
// shmget() shmctl() shmat()  shmdt()
// 获取共享内存需要约定好的key值，需要大小，两个标志位IPC_CREATE存在连接不存在创建
// IPC_EXCL单个不能用 需要连着用存在就报错
// shmctl()用于删除shm需要shmid 标志位IPC_RMID是删除shm IPC_STAT 是传递shm的数据结构 ，第三个参数
// struct shmid_ds* buf (输出型参数)接受返回值
// shmat() 挂接1.id 2.虚拟内存地址（找个固定的地址挂起）3.标志位
// shmdt() 给个虚拟地址解绑
class Shm
{
private:
    void Step(int flag) // 创建共享内存，根据不同的角色划分不同的创建功能
    {
        printf("0x%x\n", _key);
        _shmid = shmget(_key, size, flag);
        if (_shmid < 0)
        {
            EXIT_CASE("shmget");
        }
        printf("my shmid is:%d\n", _shmid);
    }
    void Creat()
    {
        Step(IPC_CREAT | IPC_EXCL | _mode);
    }
    // 挂接
    void Attach()
    {
        _vtadd = shmat(_shmid, nullptr, 0);
        if ((long)_vtadd < 0)
        {
            EXIT_CASE("shmat");
        }
        printf("shmat success\n");
    }
    // 取消挂接
    void DisAt()
    {
        int n = shmdt(_vtadd);
        if (n < 0)
        {
            EXIT_CASE("shmdt");
        }
        printf("shmdt success\n");
    }
    // 销毁根据不同角色实现不同的功能
    void Destroy()
    {
        DisAt();
        if (_usertype == "server")
        {
            int n = shmctl(_shmid, IPC_RMID, 0);
            if (n < 0)
            {
                EXIT_CASE("shmctl");
            }
        }
        printf("destory shm success\n");
    }
    void OtherGet()
    {
        Step(IPC_CREAT);
    }

public:
    Shm(const std::string &path, const int &projid, const std::string usertype) : _mode(gmode),
                                                                                  _usertype(usertype),
                                                                                  _shmid(0),
                                                                                  _vtadd(nullptr)
    {
        _key = ftok(path.c_str(), projid); // 通过输入的路径和projid生成key
        if (_key < 0)
        {
            EXIT_CASE("ftok");
        }

        if (_usertype == "server")
        {
            Creat();
        }
        else if (_usertype == "user")
        {
            OtherGet();
        }
        else
        {
            printf("please input a value both server and user\n");
            exit(1);
        }
        Attach();
    }
    key_t KeyGet()
    {
        return _key;
    }
    void *VtaddGet()
    {
        return _vtadd;
    }
    struct shmid_ds *KernelGet(struct shmid_ds *buffer)
    {
        int n = shmctl(_shmid, IPC_STAT, buffer);
        if (n < 0)
        {
            EXIT_CASE("get fail");
        }

        return buffer;
    }
    ~Shm()
    {
        if (_usertype == "server")
        {
            Destroy();
        }
        else
        {
            DisAt();
        }
    }

private:
    key_t _key;
    int _shmid;
    int _mode;
    void *_vtadd;
    std::string _usertype;
};