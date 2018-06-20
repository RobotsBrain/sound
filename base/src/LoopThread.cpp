#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <new>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/prctl.h>
#include <sys/syscall.h>

#include "base/LoopThread.h"


//#define BASE_DEBUG

#ifdef BASE_DEBUG
#define DBG(x)  x
#else
#define DBG(x)
#endif // BASE_DEBUG


namespace Base {

namespace Detail
{

template <class T>
class MemFunc
{
	typedef T ObjectT;
	typedef void (T::*FuncT)();

public:
	MemFunc(ObjectT* obj, FuncT func)
		: mObj(obj), mFunc(func)
	{
	}

	void operator()()
	{
		(mObj->*mFunc)();
	}

private:
	ObjectT*	mObj;
	FuncT		mFunc;
};

typedef MemFunc<CLoopThread> LoopThreadMemFunc;



enum ThreadState
{
    LOOPTHREAD_STOPPED,
    LOOPTHREAD_STARTING,
    LOOPTHREAD_RUNNING,
    LOOPTHREAD_OVER,
};


enum {
    LOOPTHREAD_WAIT_LOOPQUIT = -1,
    LOOPTHREAD_SIGNAL_NONE   = 0,
};

} // namespace Detail


struct CLoopThread::Impl
{
    char*           name;
    int             stacksize;
    int             priority;
    pid_t           tid;

    pthread_mutex_t     mutex;
    pthread_t           thread;
    Detail::ThreadState state;

    pthread_mutex_t sig_mtx;
    pthread_cond_t  sig_cond;
    int             sig_flag;

	Detail::LoopThreadMemFunc func;

    static void* ThreadProc(void* data)
    {
        Impl* impl = (Impl*)data;
        assert(impl);
#ifdef __ANDROID__
        impl->tid = gettid();
#else
        impl->tid = syscall(SYS_gettid);
#endif // __ANDROID__
        impl->state = Detail::LOOPTHREAD_RUNNING;
#if defined (__GLIBC__) && (__GLIBC__ >= 2) && (__GLIBC_MINOR__ >= 12)
        pthread_setname_np(impl->thread, impl->name);
#else
        prctl(PR_SET_NAME, impl->name, 0, 0, 0);
#endif
        printf("%s[%d] thread begin >>>\n", impl->name, impl->tid);
        impl->func();
        printf("%s[%d] thread end !!!\n", impl->name, impl->tid);
        impl->state = Detail::LOOPTHREAD_OVER;
        return NULL;
    }
};

/// 构造函数
CLoopThread::CLoopThread(char const* name, int stacksize, int priority)
{
    m_impl = (Impl*) operator new(sizeof(Impl));

    m_impl->name = name ? strdup(name) : NULL;
    m_impl->stacksize = stacksize;
    m_impl->priority = priority;
    m_impl->thread = 0;
    m_impl->state = Detail::LOOPTHREAD_STOPPED;
    pthread_mutex_init(&m_impl->mutex, NULL);

    m_impl->sig_flag = Detail::LOOPTHREAD_SIGNAL_NONE;
    pthread_mutex_init(&m_impl->sig_mtx, NULL);
#ifdef __ANDROID__
    pthread_cond_init(&m_impl->sig_cond, NULL);
#else
    pthread_condattr_t attr;
    pthread_condattr_init(&attr);
    pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    pthread_cond_init(&m_impl->sig_cond, &attr);
	pthread_condattr_destroy(&attr);
#endif

	new (&m_impl->func) Detail::LoopThreadMemFunc(this, &CLoopThread::ThreadProc);
}

/// 析构函数
CLoopThread::~CLoopThread()
{
	StopThread();

    pthread_cond_destroy(&m_impl->sig_cond);
    pthread_mutex_destroy(&m_impl->sig_mtx);
    pthread_mutex_destroy(&m_impl->mutex);

    free(m_impl->name);

	using Detail::LoopThreadMemFunc;
	m_impl->func.~LoopThreadMemFunc();

	operator delete(m_impl);
}

/// 启动线程
bool CLoopThread::StartThread()
{
    //printf("CLoopThread::StartThread() >>>\n");

    if (m_impl->state != Detail::LOOPTHREAD_STOPPED)
    {
        printf("%s[%d] has been started!\n", m_impl->name, m_impl->tid);
        return true;
    }

    bool succ = true;

    pthread_mutex_lock(&m_impl->mutex);

    if (m_impl->state == Detail::LOOPTHREAD_STOPPED)
    {
        pthread_mutex_lock(&m_impl->sig_mtx);
        m_impl->sig_flag = Detail::LOOPTHREAD_SIGNAL_NONE;
        pthread_mutex_unlock(&m_impl->sig_mtx);

        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, m_impl->stacksize);

        if (m_impl->priority >= 0)
        {
            pthread_attr_setschedpolicy(&attr, SCHED_FIFO);

            struct sched_param param;
            pthread_attr_getschedparam(&attr, &param);
            param.sched_priority = m_impl->priority;
            pthread_attr_setschedparam(&attr, &param);
            printf("set thread(%s) priority to %d\n", m_impl->name, m_impl->priority);
        }

        assert(m_impl->thread == 0);
        if (pthread_create(&m_impl->thread, &attr, CLoopThread::Impl::ThreadProc, m_impl) == 0)
        {
            m_impl->state = Detail::LOOPTHREAD_STARTING;
        }
        else
        {
            printf("[%s] create pthread error!\n", m_impl->name);
            succ = false;
        }

        pthread_attr_destroy(&attr);
    }

    pthread_mutex_unlock(&m_impl->mutex);
	return succ;
}

/// 停止线程
bool CLoopThread::StopThread()
{
    //printf("CLoopThread::StopThread() >>>\n");

	if (m_impl->state == Detail::LOOPTHREAD_STOPPED)
	{
        printf("%s[%d] has been stopped!\n", m_impl->name, m_impl->tid);
		return true;
	}

    pthread_mutex_lock(&m_impl->mutex);

	if (m_impl->state != Detail::LOOPTHREAD_STOPPED)
	{
	    // signal quit message to thread loop
        pthread_mutex_lock(&m_impl->sig_mtx);
        m_impl->sig_flag = Detail::LOOPTHREAD_WAIT_LOOPQUIT;
        pthread_cond_signal(&m_impl->sig_cond);
        pthread_mutex_unlock(&m_impl->sig_mtx);

        printf("waiting thread(%s)[%d] over...\n", m_impl->name, m_impl->tid);
        pthread_join(m_impl->thread, NULL);
        m_impl->thread = 0;

        m_impl->state = Detail::LOOPTHREAD_STOPPED;
	}

    pthread_mutex_unlock(&m_impl->mutex);
	return true;
}

/// 发送信号给循环体, 由 Wait 接收并清除
/// \param [in] signal >0, 由派生类定义
bool CLoopThread::SignalThread(int signal)
{
    if (signal < 0)
    {
        printf("signal can not be less then zero!\n");
        return false;
    }

    pthread_mutex_lock(&m_impl->sig_mtx);
    if (Detail::LOOPTHREAD_WAIT_LOOPQUIT != m_impl->sig_flag) {
        m_impl->sig_flag = signal;
        pthread_cond_signal(&m_impl->sig_cond);
    }
    pthread_mutex_unlock(&m_impl->sig_mtx);

    return true;
}

/// 等待信号, 收到信号后, 清除信号标志
/// \param [in] timeout 超时时间(ms)
/// \retval >0 派生类定义的信号; -1 线程结束信号;
int CLoopThread::WaitSignal(int timeout)
{
    timespec ts_expired = {0};
    if (timeout >= 0)
    {
        timespec ts_now = {0};
        clock_gettime(CLOCK_MONOTONIC, &ts_now);

        int timeout_sec = timeout / 1000;
        int timeout_nsec = (timeout - timeout_sec * 1000) * 1000000;
        int expired_nsec = ts_now.tv_nsec + timeout_nsec;   // 到期纳秒数
        int expired_nsec_sec  = expired_nsec / 1000000000L;     // 到期纳秒数中的秒数
        ts_expired.tv_sec = ts_now.tv_sec + timeout_sec + expired_nsec_sec;
        ts_expired.tv_nsec = expired_nsec % 1000000000L;

        DBG(printf("[%s] now(%d.%09ds), timeout(%d.%09ds), expired(%d.%09ds)\n",
                m_impl->name,
                ts_now.tv_sec, ts_now.tv_nsec,
                timeout_sec, timeout_nsec,
                ts_expired.tv_sec, ts_expired.tv_nsec));
    }

    pthread_mutex_lock(&m_impl->sig_mtx);
    if (m_impl->sig_flag == Detail::LOOPTHREAD_SIGNAL_NONE)
    {
        DBG(printf("waiting [%s] timeout(%dms) ...\n", m_impl->name, timeout));
        if (timeout >= 0)
        {
#ifdef __ANDROID__
            pthread_cond_timedwait_monotonic(&m_impl->sig_cond, &m_impl->sig_mtx, &ts_expired);
#else
            pthread_cond_timedwait(&m_impl->sig_cond, &m_impl->sig_mtx, &ts_expired);
#endif
        }
        else
        {
            pthread_cond_wait(&m_impl->sig_cond, &m_impl->sig_mtx);
        }
        DBG(printf("exit wait [%s] sig_flag(%d)\n", m_impl->name, m_impl->sig_flag));
    }
    int retval = m_impl->sig_flag;
    m_impl->sig_flag = Detail::LOOPTHREAD_SIGNAL_NONE;
    pthread_mutex_unlock(&m_impl->sig_mtx);

    DBG(printf("[%s] retval(%d)\n", m_impl->name, retval));
    return retval;
}


} // namespace Base

