#include "EventLoopThread.h"
#include "EventLoop.h"
#include <functional>
#include <thread>
#include <mutex>

EventLoopThread::EventLoopThread()
	: m_pLoop(NULL),     //�����̴߳�����EventLoop*
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
		//����ϵͳ�жϣ���������������ٻ��ѣ���Ҫ�ٴμ�⣬loop�Ƿ񴴽����
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
		m_cvCond.notify_one(); //�߳��Ѿ���ɣ�֪ͨ���߳�
	}
	loop.loop();
}