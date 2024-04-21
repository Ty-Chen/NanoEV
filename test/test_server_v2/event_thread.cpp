#include <conio.h>
#include <atlbase.h>
#include <atltrace.h>
#include <cassert>
#include <signal.h>
#include <atomic>
#include "event_thread.h"

//------------------------------------------------------------------------------

class breakLoopTask : public EventTask
{
public:
    void run(nanoev_loop *loop, bool run)
    {
        if (run)
        {
            nanoev_loop_break(loop);
        }
    }
};

//------------------------------------------------------------------------------

EventThread::EventThread()
{
    InitializeCriticalSection(&m_cs);
    m_loop = NULL;
    m_async = NULL;
    m_thread = NULL;
    m_threadID = 0;
}

EventThread::~EventThread()
{
    assert(m_async == NULL);
    assert(m_loop == NULL);
    if (m_thread)
    {
        CloseHandle(m_thread);
        m_thread = NULL;
    }
    DeleteCriticalSection(&m_cs);
}

bool EventThread::start()
{
    bool retCode = false;

    if (m_thread != NULL)
        goto Exit;

    m_loop = nanoev_loop_new(this);
    if (!m_loop)
        goto Exit;

    m_async = nanoev_event_new(nanoev_event_async, m_loop, this);
    if (!m_async)
        goto Exit;
    nanoev_async_start(m_async, __onAsync);

    m_thread = (HANDLE)_beginthreadex(NULL, 0, __threadProc, this, 0, NULL);
    if (!m_thread)
        goto Exit;

    retCode = true;
Exit:
    if (!retCode)
    {
        if (m_async)
        {
            nanoev_event_free(m_async);
            m_async = NULL;
        }
        if (m_loop)
        {
            nanoev_loop_free(m_loop);
            m_loop = NULL;
        }
    }
    return retCode;
}

void EventThread::stop()
{
    if (m_thread)
    {
        queueTask(new breakLoopTask);
        WaitForSingleObject(m_thread, INFINITE);
    }

    __processTasks(false);

    if (m_async)
    {
        nanoev_event_free(m_async);
        m_async = NULL;
    }

    if (m_loop)
    {
        nanoev_loop_free(m_loop);
        m_loop = NULL;
    }
}

void EventThread::queueTask(EventTask *task)
{
    assert(task);
    assert(isRunning());

    EnterCriticalSection(&m_cs);
    m_taskQueue.push(task);
    LeaveCriticalSection(&m_cs);

    nanoev_async_send(m_async);
}

bool EventThread::isRunning()
{
    bool isRunning = false;
    if (m_thread != NULL && WaitForSingleObject(m_thread, 0) == WAIT_TIMEOUT)
    {
        isRunning = true;
    }
    return isRunning;
}

void EventThread::__onAsync(nanoev_event *async)
{
    EventThread *pThis = (EventThread*)nanoev_event_userdata(async);
    pThis->__processTasks(true);
}

unsigned int EventThread::__threadProc(void *arg)
{
    EventThread *pThis = (EventThread*)arg;

    pThis->m_threadID = GetCurrentThreadId();

    // Setting a higher resolution can improve the accuracy of time-out intervals in wait functions.
    timeBeginPeriod(1);

    int retCode = nanoev_loop_run(pThis->m_loop);

    timeEndPeriod(1);

    pThis->m_threadID = 0;

    return 0;
}

void EventThread::__processTasks(bool run)
{
    EnterCriticalSection(&m_cs);
    while(m_taskQueue.size())
    {
        EventTask *task = m_taskQueue.front();
        m_taskQueue.pop();
        // Unlock
        LeaveCriticalSection(&m_cs);
        // Callback
        task->run(m_loop, run);
        delete task;
        // Lock
        EnterCriticalSection(&m_cs);
    }
    LeaveCriticalSection(&m_cs);
}
