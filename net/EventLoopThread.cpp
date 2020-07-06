#include "EventLoopThread.h"
#include "EventLoop.h"
#include <functional>
#include <thread>
#include <mutex>

EventLoopThread::EventLoopThread()
	: m_pLoop(NULL),     //由子线程创建的EventLoop*
	m_bExiting(false),	
	m_Thread(),
	m_Mutex(),
	m_cvCond()
{
}

EventLoopThread::~EventLoopThread()
{
	m_bExiting = true;
	m_pLoop->quit();
	m_Thread.join();
}

EventLoop* EventLoopThread::startLoop()
{
	m_Thread = std::thread(std::bind(&EventLoopThread::threadFunc, this));
	{
		std::unique_lock<std::mutex> lock(m_Mutex);
		//由于系统中断，条件变量存在虚假唤醒，需要再次检测，loop是否创建完成
		while (m_pLoop == NULL)
		{
			m_cvCond.wait(lock); 
		}
	}
	return m_pLoop;
}

void EventLoopThread::threadFunc()
{
	EventLoop	loop;
	{
		std::unique_lock<std::mutex>	lock(m_Mutex);
		m_pLoop = &loop;
		m_cvCond.notify_one(); //线程已经完成，通知主线程
	}
	loop.loop();
}