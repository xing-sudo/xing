#include <iostream>
#include <cstdint>
#include <vector>
#include <assert.h>
#include <cstring>
#include <ctime>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <functional>
#include <sys/epoll.h>
// 宏函数定义
#define INF 0
#define DBG 1
#define ERR 2
#define LOG_LEVEL DBG

// strftime函数
// fprintf函数
#define LOG(level, format, ...)                                                                                       \
    do                                                                                                                \
    {                                                                                                                 \
        if (level < LOG_LEVEL)                                                                                        \
            break;                                                                                                    \
        time_t t = time(nullptr);                                                                                     \
        struct tm *ltm = localtime(&t);                                                                               \
        char tmp[32] = {0};                                                                                           \
        strftime(tmp, 31, "%H:%M:%S", ltm);                                                                           \
        fprintf(stdout, "[%p %s %s:%d]" format "\n", (void *)pthread_self(), tmp, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (0)

#define INF_LOG(format, ...) LOG(INF, format, ##__VA_ARGS__)
#define DBG_LOG(format, ...) LOG(DBG, format, ##__VA_ARGS__)
#define ERR_LOG(format, ...) LOG(ERR, format, ##__VA_ARGS__)

#define BUFFER_SIZE 1024
class Buffer
{
private:
    uint64_t _write_pos;     // 写位置
    uint64_t _read_pos;      // 读位置
    std::vector<char> _data; // 容器

public:
    Buffer() : _write_pos(0), _read_pos(0), _data(BUFFER_SIZE) {}
    Buffer &operator=(const Buffer &other) // 赋值重载
    {
        if (this != &other)
        {
            _write_pos = other._write_pos;
            _read_pos = other._read_pos;
            _data = other._data; // vector的赋值操作会自动处理内存
        }
        return *this;
        // 实际上这里的赋值操作应该是深拷贝
        //_data=new vector<char>(*other._data); // 通过复制构造函数创建一个新的vector对象
    }
    char *Head() // 获取容器头位置
    {
        return &*_data.begin(); // data.begin()返回一个指向vector第一个元素的迭代器，&*操作符将其转换为指针
    }
    char *GetWritePos()
    {
        return Head() + _write_pos; // 写位置指针
    }
    char *GetReadPos()
    {
        return Head() + _read_pos; // 读位置指针
    }
    uint64_t TailSpace()
    {
        return _data.size() - _write_pos; // 尾部剩余空间
    }
    uint64_t HeadSpace()
    {
        return _read_pos; // 头部剩余空间
    }
    uint64_t ReadAbleSize()
    {
        return _write_pos - _read_pos; // 可读数据大小
    }
    // 移动读位置
    void MoveReadPos(uint64_t len)
    {
        assert(len <= ReadAbleSize());
        _read_pos += len;
    }
    // 移动写位置
    void MoveWritePos(uint64_t len)
    {
        assert(len <= TailSpace());
        _write_pos += len;
    }
    // 确保有足够的空间写入数据
    void EnsureSpace(uint64_t len)
    {
        if (TailSpace() >= len) // 如果尾部空间足够，直接返回
        {
            return;
        }
        // 如果头部和尾部空间之和足够，将可读数据移动到头部
        // copy()
        if (HeadSpace() + TailSpace() >= len)
        {
            uint64_t readable_size = ReadAbleSize();                       // 保留可读数据大小
            std::copy(GetReadPos(), GetReadPos() + readable_size, Head()); // 将可读数据移动到头部
            _read_pos = 0;                                                 // 更新读位置
            _write_pos = readable_size;                                    // 更新写位置
        }
        else
        {
            _data.resize(_write_pos + len); // 否则，扩展容器大小
        }
    }
    void Write(const void *data, uint64_t len)
    {
        if (len == 0)
        {
            return;
        }

        EnsureSpace(len);                                                       // 确保有足够空间写入数据
        std::copy((const char *)data, (const char *)data + len, GetWritePos()); // 将数据写入写位置
        MoveWritePos(len);                                                      // 更新写位置
    }
    // 写入字符串
    void Write(const std::string &str)
    {
        Write(str.c_str(), str.size());
    }
    // 读取数据
    void Read(void *buff, uint64_t len)
    {
        assert(len <= ReadAbleSize());
        std::copy(GetReadPos(), GetReadPos() + len, (char *)buff);
        MoveReadPos(len);
    }
    // 读取指定长度的数据并返回字符串
    std::string ReadAsstring(uint64_t len)
    {
        assert(len <= ReadAbleSize());
        std::string str;
        str.resize(len);
        Read(&str[0], len);
        return str;
    }
    // 查找CRLF位置
    // memchr()
    char *FindCRLF()
    {
        char *pos = (char *)memchr(GetReadPos(), '\n', ReadAbleSize()); // 查找换行符
        return pos;
    }
    // 读取一行数据，直到CRLF
    std::string Getline()
    {
        char *pos = FindCRLF();
        if (pos == nullptr)
        {
            return "";
        }
        return ReadAsstring(pos - GetReadPos() + 1);
    }
    // 清空缓冲区
    void clear()
    {
        _write_pos = 0;
        _read_pos = 0;
    }
};

#define MAX_LISTEN 1024
// sockaddr_in,sockaddr
// setsockopt
// EAGAIN,EINTR
// fctnl
class Socket
{
private:
    int _sockfd; // 套接字文件描述符
public:
    Socket() : _sockfd(-1) {}
    Socket(int sockfd) : _sockfd(sockfd) {}
    ~Socket()
    {
        Close();
    }
    int GetFd() const
    {
        return _sockfd;
    }
    // 创建套接字
    bool Create()
    {
        _sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (_sockfd < 0)
        {
            ERR_LOG("create socket failed!!");
            return false;
        }
        return true;
    }
    // 绑定地址和端口
    bool Bind(const std::string &ip, uint16_t port)
    {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(ip.c_str());
        socklen_t len = sizeof(addr);
        int ret = bind(_sockfd, (struct sockaddr *)&addr, len);
        if (ret < 0)
        {
            ERR_LOG("bind,failed!!");
            return false;
        }

        return true;
    }
    // 开始监听
    bool Listen()
    {
        int ret = listen(_sockfd, MAX_LISTEN);
        if (ret < 0)
        {
            ERR_LOG("listen failed!!");
            return false;
        }
        return true;
    }
    // 向服务器发起连接
    bool Connect(const std::string &ip, uint16_t port)
    {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port + htons(port);
        addr.sin_addr.s_addr = inet_addr(ip.c_str());
        socklen_t len = sizeof(addr);
        int ret = connect(_sockfd, (struct sockaddr *)&addr, len);
        if (ret < 0)
        {
            ERR_LOG("connect failed!!");
            return false;
        }
        return true;
    }
    // 获取新连接
    int Accept()
    {
        int newfd = accept(_sockfd, nullptr, nullptr);
        if (newfd < 0)
        {
            ERR_LOG("accept failed!!");
            return -1;
        }
        return newfd;
    }
    // 接收数据
    ssize_t Recv(void *buff, size_t len, int flag = 0)
    {
        ssize_t ret = recv(_sockfd, buff, len, flag);
        if (ret <= 0)
        {
            // EAGAIN表示非阻塞套接字没有数据可读
            // EINTR表示系统调用被信号中断，这两种情况都不是错误，可以继续尝试读取数据
            if (errno == EAGAIN || errno == EINTR)
            {
                return 0;
            }
            ERR_LOG("recv failed!!");
            return -1;
        }
        return ret;
    }
    ssize_t NonBlockRecv(void *buff, size_t len)
    {
        return Recv(buff, len, MSG_DONTWAIT);
        // MSG_DONTWAIT标志表示非阻塞接收，如果没有数据可读，recv函数会立即返回而不是阻塞等待
    }
    // 发送数据
    ssize_t Send(const void *buff, size_t len, int flag = 0)
    {
        ssize_t ret = send(_sockfd, buff, len, flag);
        if (ret < 0)
        {
            if (errno == EAGAIN || errno == EINTR)
            {
                return 0;
            }
            ERR_LOG("send failed!!");
            return -1;
        }
        return ret; // 返回实际发送的字节数
    }
    ssize_t NonBlockSend(const void *buff, size_t len)
    {
        return Send(buff, len, MSG_DONTWAIT);
    }
    // 关闭套接字
    void Close()
    {
        if (_sockfd != -1)
        {
            close(_sockfd);
            _sockfd = -1;
        }
    }
    // 设置套接字为非阻塞模式
    void NonBlock()
    {
        int flag = fcntl(_sockfd, F_GETFL, 0);
        fcntl(_sockfd, F_SETFL, flag | O_NONBLOCK);
    }
    // 设置套接字地址复用
    void ReuseAddr()
    {
        int val = 1;
        // setsockopt函数用于设置套接字选项，参数说明如下：
        // __fd：套接字文件描述符，指定要设置选项的套接字。
        // __level：选项所在的协议层，通常使用SOL_SOCKET表示套接字级别的选项。
        // __optname：要设置的选项名称，例如SO_REUSEADDR表示允许重用本地地址，SO_REUSEPORT表示允许多个套接字绑定到同一个端口。
        // __optval：指向包含选项值的缓冲区的指针，这里传入&val表示设置选项的值为1。
        // __optlen：选项值的长度，这里传入sizeof(int)表示选项值的长度为一个整数的大小。
        setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int));
        val = 1;
        setsockopt(_sockfd, SOL_SOCKET, SO_REUSEPORT, &val, sizeof(int));
    }
    // 创建服务端连接
    bool CreateServer(uint64_t port, const std::string &ip = "0.0.0.0", bool block_flag = false)
    {
        // 创建，绑定，监听，非阻塞，地址复用
        if (Create() == false)
        {
            return false;
        }
        if (Bind(ip, port) == false)
        {
            return false;
        }
        if (Listen() == false)
        {
            return false;
        }
        if (block_flag)
            NonBlock();
        ReuseAddr();
    }
    // 创建客户端连接
    bool CreateClient(const std::string &ip, uint64_t port, bool block_flag = false)
    {
        // 创建，连接
        if (Create() == false)
        {
            return false;
        }
        if (Connect(ip, port) == false)
        {
            return false;
        }
        return true;
    }
};
class Poller;
class EventLoop;
// 对描述符需要监控的事件和触发的事件进行管理(可读，可写,关闭，错误，任意)
class Channel
{
    /*
    EPOLLIN：表示对应的文件描述符可以读（包括对端SOCKET正常关闭）；
    EPOLLOUT：表示对应的文件描述符可以写；
    EPOLLRDHUP：表示对应的文件描述符读关闭；
    EPOLLPRI：表示对应的文件描述符有紧急数据可读；
    EPOLLHUP：表示对应的文件描述符被挂断；
    EPOLLERR：表示对应的文件描述符发生错误；
    */
private:
    int fd; // 监控的文件描述符
    EventLoop *_loop;
    uint32_t _events;  // 监控的事件类型
    uint32_t _revents; // 触发的事件类型
    using Eventcallback = std::function<void()>;
    Eventcallback _read_callback;  // 可读事件回调函数
    Eventcallback _write_callback; // 可写事件回调函数
    Eventcallback _close_callback; // 关闭事件回调函数
    Eventcallback _error_callback; // 错误事件回调函数
    Eventcallback _event_callback; // 任意事件回调函数
public:
    Channel() {}
    int GetEvents() const
    {
        return _events;
    } // 获取监控的事件类型
    int GetFd() const
    {
        return fd;
    } // 获取监控的文件描述符
    void SetRevents(uint32_t revents)
    {
        _revents = revents;
    } // 设置触发的事件类型
    void SetReadCallBack(const Eventcallback &cb)
    {
        _read_callback = cb;
    } // 设置可读事件回调函数
    void SetWriteCallBack(const Eventcallback &cb)
    {
        _write_callback = cb;
    } // 设置可写事件回调函数
    void SetCloseCallBack(const Eventcallback &cb)
    {
        _close_callback = cb;
    } // 设置关闭事件回调函数
    void SetErrorCallBack(const Eventcallback &cb)
    {
        _error_callback = cb;
    } // 设置错误事件回调函数
    void SetEventCallBack(const Eventcallback &cb)
    {
        _event_callback = cb;
    } // 设置任意事件回调函数
    void EnableRead()
    {
        _events |= EPOLLIN;
    } // 开启可读事件监控
    void EnableWrite()
    {
        _events |= EPOLLOUT;
    } // 开启可写事件监控
    void DisableRead() { _events &= ~EPOLLIN; } // 关闭可读事件监控
    void DisableWrite()
    {
        _events &= ~EPOLLOUT;
    } // 关闭可写事件监控
    void DisableAll()
    {
        _events = 0;
    } // 关闭所有事件监控
    bool ReadAble()
    {
        return _events & EPOLLIN;
    } // 是否可读
    bool WriteAble()
    {
        return _events & EPOLLOUT;
    } // 是否可写
    void Remove() {} // 移除监控
    void Update() {} // 更新监控事件
    void HandleEvent()
    {
        if ((_revents & EPOLLIN) || (_revents & EPOLLRDHUP) || (_revents & EPOLLPRI))
        {
            // 可读事件触发，EPOLLRDHUP表示对端关闭连接，EPOLLPRI表示有紧急数据可读
            if (_read_callback)
            {
                _read_callback();
            }
        }
        if (_revents & EPOLLOUT)
        {
            if (_write_callback)
            {
                _write_callback();
            }
        }
        else if (_revents & EPOLLHUP)
        {
            if (_close_callback)
            {
                _close_callback();
            }
        }
        else if (_revents & EPOLLERR)
        {
            if (_error_callback)
            {
                _error_callback();
            }
        }
        // 无论什么事件都要触发任意事件回调函数，刷新活跃度
        if (_event_callback)
        {
            _event_callback();
        }
    } // 处理触发的事件
};
#define MAX_EPOLLEVENTS 1024
// 通过EPOLL实现对描述符的封装
// epoll_ctl函数用于控制epoll实例的事件注册、修改和删除，参数说明如下：
// __epfd：epoll实例的文件描述符，表示要操作的epoll实例。
// __op：操作类型，表示要执行的操作，可以是以下值之一：
// EPOLL_CTL_ADD：向epoll实例中添加一个新的文件描述符和事件类型的监控。
// EPOLL_CTL_MOD：修改已经注册的文件描述符的事件类型监控。
// EPOLL_CTL_DEL：从epoll实例中删除一个已经注册的文件描述符的监控。
// __fd：要操作的文件描述符，表示要添加、修改或删除监控的文件描述符。
// __event：指向一个epoll_event结构体的指针，包含要监控的事件类型和相关数据。epoll_event结构体

// epoll_create函数用于创建一个新的epoll实例，参数说明如下：
//  __size：指定epoll实例能够监控的最大文件描述符数量。这个参数在现代Linux内核中已经被忽略，但仍然需要传入一个正整数

// epoll_wait函数用于等待epoll实例中注册的事件发生，参数说明如下：
// __epfd：epoll实例的文件描述符，表示要等待事件的epoll实例。
// __events：指向一个epoll_event结构体数组的指针，用于存储发生事件的文件描述符和相关数据。数组的大小应该至少为maxevents参数指定的值。
// __maxevents：指定events数组的大小，表示events数组中可以存储的最大事件数量。这个参数决定了epoll_wait函数一次能够返回的最大事件数量。
// __timeout：指定等待事件的超时时间，单位为毫秒。可以设置为以下值之一：
// -1：表示无限等待，直到有事件发生。
// 0：表示立即返回，不等待事件发生
//返回值：成功时返回发生事件的文件描述符数量，失败时返回-1，并设置errno以指示错误原因。
class Poller
{
private:
    int _epfd;
    struct epoll_event _events[MAX_EPOLLEVENTS];  // 存储触发事件的数组
    std::unordered_map<int, Channel *> _channels; // 文件描述符到Channel对象的映射
private:
    void Updata(Channel *channel, int op)
    {
        int fd = channel->GetFd();
        struct epoll_event ev;
        ev.data.fd = fd;
        ev.events = channel->GetEvents();
        int ret = epoll_ctl(_epfd, op, fd, &ev);
        if (ret < 0)
        {
            ERR_LOG("epoll_ctl failed!!");
        }
        return;
    }
    // 判断一个描述符是否添加了poller管理
    bool HasChannel(Channel *channel)
    {
        auto it = _channels.find(channel->GetFd());
        if (it == _channels.end())
        {
            return false;
        }
        return true;
    }

public:
    Poller()
    {
        _epfd == epoll_create(MAX_EPOLLEVENTS);
        if (_epfd < 0)
        {
            ERR_LOG("epoll_create failed!!");
            abort();
        }
    }
    void UpdataEvent(Channel *channel)
    {
        bool exist = HasChannel(channel);
        if (exist == false)
        {
            // 不存在添加
            _channels.insert(std::make_pair(channel->GetFd(), channel));
            return Updata(channel, EPOLL_CTL_ADD);
        }
    }
    //删除事件
    void RemoveEvent(Channel *channel)
    {
        auto it = _channels.find(channel->GetFd());
        if (it != _channels.end())
        {
            _channels.erase(it);
        }
        return Updata(channel, EPOLL_CTL_DEL);
    }
    //开始监控并返回活跃连接
    void Moniter(std::vector<Channel *> *active)
    {
        int nfds=epoll_wait(_epfd, _events, MAX_EPOLLEVENTS, -1);
        if(nfds<0)
        {
            if(errno=EINTR)
            {
                return;
            }
            ERR_LOG("epoll_wait failed!!");
            abort();
        }
        for(int i=0;i<nfds;i++)
        {
            auto it =_channels.find(_events[i].data.fd);
            assert(it!=_channels.end());
            it->second->SetRevents(_events[i].events);//设置就绪事件
            active->push_back(it->second);
        }
    }
};
