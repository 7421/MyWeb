#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

#include <vector>
#include <functional>
#include <memory>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool
{
public:
	EventLoopThreadPool(EventLoop* baseLoop);
	~EventLoopThreadPool();

	EventLoopThreadPool(const EventLoopThreadPool&) = delete;
	void operator=(const EventLoopThreadPool&) = delete;

	void setThreadNum(int numThreads)
	{
		m_nNumThreads = numThreads;
	}
	void start();
	EventLoop* getNextLoop();

private:
	EventLoop* m_BaseLoop;						//基事件驱动循环
	bool       m_bStarted;						//线程池开始标志
	int		   m_nNumThreads;					//线程池线程数目
	int		   m_nNext;							//轮询任务安排，线程编号
	std::vector<std::shared_ptr<EventLoopThread>> m_vecThreads; //线程池所管理的线程对象
	std::vector<EventLoop*>						  m_vecLoops;	//线程池所管理的事件驱动循环
};