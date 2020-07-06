#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>

#include "EventLoop.h"

class EventLoopThread
{
public:
	EventLoopThread();
	~EventLoopThread();

	EventLoopThread(const EventLoopThread&) = delete;
	void operator=(const EventLoopThread&) = delete;
	
	EventLoop* startLoop();
private:
	void threadFunc();

	EventLoop*				m_pLoop;	//EventLoopThread所拥有的eventLoop
	bool					m_bExiting;	//EventLoopThread线程退出标志	
	std::thread				m_Thread;	//当前线程
	std::mutex				m_Mutex;	//互斥锁
	std::condition_variable m_cvCond;	//条件变量，用于初始化线程
};