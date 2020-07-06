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

	EventLoop*				m_pLoop;	//EventLoopThread��ӵ�е�eventLoop
	bool					m_bExiting;	//EventLoopThread�߳��˳���־	
	std::thread				m_Thread;	//��ǰ�߳�
	std::mutex				m_Mutex;	//������
	std::condition_variable m_cvCond;	//�������������ڳ�ʼ���߳�
};