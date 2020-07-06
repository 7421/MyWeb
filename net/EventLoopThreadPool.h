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
	EventLoop* m_BaseLoop;						//���¼�����ѭ��
	bool       m_bStarted;						//�̳߳ؿ�ʼ��־
	int		   m_nNumThreads;					//�̳߳��߳���Ŀ
	int		   m_nNext;							//��ѯ�����ţ��̱߳��
	std::vector<std::shared_ptr<EventLoopThread>> m_vecThreads; //�̳߳���������̶߳���
	std::vector<EventLoop*>						  m_vecLoops;	//�̳߳���������¼�����ѭ��
};