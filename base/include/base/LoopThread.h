
#ifndef __BASE_LOOP_THREAD_H__
#define __BASE_LOOP_THREAD_H__


namespace Base {

/// 带循环体线程基类
class CLoopThread
{
public:
	/// 构造函数
    CLoopThread(char const* name, int stacksize = 1024 * 1024, int priority = -1);

	/// 析构函数
	virtual ~CLoopThread();

	/// 启动线程
	bool StartThread();

	/// 停止线程
	bool StopThread();

protected:
    /// 发送信号给循环体, 由 Wait 接收并清除
    /// \param [in] signal 必须非负; 0表示无信号仅唤醒线程; >0 由派生类定义
	bool SignalThread(int signal);

	/// 等待信号, 收到信号后, 清除信号标志
	/// \param [in] timeout 超时时间(ms), <0 表示不会超时退出
	/// \retval >0 派生类定义的信号; 0无信号或超时被唤醒; -1 线程结束信号
	int WaitSignal(int timeout = -1);

	/// 线程循环体
	virtual void ThreadProc() = 0;

private:
    CLoopThread(CLoopThread const&);
    CLoopThread& operator=(CLoopThread const&);

private:
	struct Impl;
	Impl* m_impl;
};

} // namespace Base


#endif // __BASE_LOOP_THREAD_H__

