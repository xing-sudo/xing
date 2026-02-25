#include <iostream>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <functional>
#include <memory>
#include <unistd.h>

using Taskfunc = std::function<void()>; // 定时任务
using Release = std::function<void()>;  // 定时器对象释放函数
class Timetask
{
public:
    Timetask(uint64_t id, uint32_t delay, const Taskfunc &cb) : _id(id), _timeout(delay), _cancel(false), _func(cb) {}

    ~Timetask()
    {
        if (_cancel == false) // 定时任务没有被取消,则执行定时任务
        {
            _func();
        }
        _release();
    }
    // 设置是否取消
    void cancel()
    {
        _cancel = true;
    }
    // 设置释放函数
    void setRelease(const Release &rel)
    {
        _release = rel;
    }
    // 获取定时任务推迟时间
    uint32_t Delaytime()
    {
        return _timeout;
    }

private:
    uint64_t _id;      // 定时器任务id
    uint32_t _timeout; // 定时任务的推迟时间
    bool _cancel;      // 定时任务是否被取消
    Taskfunc _func;    // 定时器对象要执行的任务
    Release _release;  // timewheel释放该定时器对象时要执行的函数
};

class Timewheel
{
public:
    Timewheel() : _tick(0), _capacity(60), _wheel(_capacity)
    {
    }

    void TimerAdd(uint64_t id, uint32_t delay, const Taskfunc &cb)
    {
        ShPtrTask pt(new Timetask(id, delay, cb));
        pt->setRelease(std::bind(&Timewheel::RemoveTimer, this, id)); // 设置释放函数
        int pos = (_tick + delay) % _capacity;                        // 计算该定时器对象应该放到时间轮的哪个位置
        _wheel[pos].push_back(pt);                                    // 放入时间轮
        _timer[id] = WeakTask(pt);                                    // 存储弱引用
    }
    void TimerRefresh(uint64_t id)
    {
        auto it = _timer.find(id);
        if (it == _timer.end())
        {
            return;
        }
        ShPtrTask pt = it->second.lock();
        int delay = pt->Delaytime();
        int pos = (_tick + delay) % _capacity; // 计算该定时器对象应该放到时间轮的哪个位置
        _wheel[pos].push_back(pt);             // 放入时间轮
    }
    void TimerCancel(uint64_t id)
    {
        auto it = _timer.find(id);
        if (it == _timer.end())
        {
            return;
        }
        ShPtrTask pt = it->second.lock();
        if (pt)
            pt->cancel();
    }
    void Run(){
        _tick = (_tick + 1) % _capacity; // 秒针走一步
        _wheel[_tick].clear();          // 释放该位置的所有定时器对象
    }
private:
    void RemoveTimer(uint64_t id)
    {
        auto it = _timer.find(id);
        if (it != _timer.end())
        {
            _timer.erase(it);
        }
    }

private:
    using WeakTask = std::weak_ptr<Timetask>;
    using ShPtrTask = std::shared_ptr<Timetask>;
    int _tick;                                     /// 秒针走到哪释放哪
    int _capacity;                                 // 最大延迟时间
    std::vector<std::vector<ShPtrTask>> _wheel;    // 时间轮
    std::unordered_map<uint64_t, WeakTask> _timer; // 定时器id到定时器对象的映射
};