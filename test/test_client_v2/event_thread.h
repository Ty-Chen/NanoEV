#ifndef __EVENT_THREAD_H__
#define __EVENT_THREAD_H__

#include "nanoev.hpp"
#include <queue>

//------------------------------------------------------------------------------

class EventTask 
{
public:
    virtual ~EventTask() {};
    virtual void run(nanoev_loop *loop, bool run) = 0;
};

typedef void (*EventTaskFunc)(nanoev_loop *loop, void *ctx, bool run);

class SimpleEventTask : public EventTask
{
    EventTaskFunc m_func;
    void *m_ctx;

public:
    SimpleEventTask(EventTaskFunc func, void *ctx)
    {
        m_func = func;
        m_ctx = ctx;
    }
    void run(nanoev_loop *loop, bool run)
    {
        m_func(loop, m_ctx, run);
    }
};

//------------------------------------------------------------------------------

class EventThread
{
    typedef std::queue<EventTask*> TaskQueue;

    CRITICAL_SECTION m_cs;
    nanoev_loop *m_loop;
    nanoev_event *m_async;
    TaskQueue m_taskQueue;    
    HANDLE m_thread;
    DWORD m_threadID;

public:
    EventThread();
    ~EventThread();

    bool start();
    void stop();
    
    void queueTask(EventTask *task);
    bool isRunning();

    bool runInEventThread()
    {
        return GetCurrentThreadId() == m_threadID;
    }

private:
    static void __onAsync(nanoev_event *async);
    static unsigned int __stdcall __threadProc(void *arg);
    void __processTasks(bool run);
};

#endif // !__EVENT_THREAD_H__
