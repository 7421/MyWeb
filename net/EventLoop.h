#pragma once

#include "../base/Timestamp.h"

#include <thread>
#include <functional>
#include <mutex>
#include <vector>

class EPoller;
class Channel;

class EventLoop
{
public:
	typedef std::function<void()> Functor;

	EventLoop();
	~EventLoop();

	EventLoop(const EventLoop&) = delete;
	EventLoop& operator=(const EventLoop&) = delete;

	void loop();

	void quit();

	Timestamp pollReturnTime() const { return m_tPollReturnTime; }
	
	//����û��ڵ�ǰIO�̵߳�������������ص���ͬ������
	//����û��������̵߳���runInLoop��cb�ᱻ�������IO�̻߳ᱻ�������������Functor
	void runInLoop(const Functor& cb);

	void queueInLoop(const Functor& cb);
	
	void wakeup();
	void updateChannel(Channel* channel);
	void removeChannel(Channel* channel);


	void assertInLoopThread();
	bool isInLoopThread() const
	{
		return m_spThreadId == std::this_thread::get_id();
	}
private:
	int createEventfd();

	void handleRead();
	void doPendingFunctors();
	
	bool						m_bQuit;					//ֹͣ��־������ֹͣ�¼�����ѭ��EventLoop
	bool						m_bLooping;				//�¼�����ѭ��EventLoop���б�־
	bool						m_bCallingPendingFunctors;  //���ڴ����¼���־
	const std::thread::id		m_spThreadId;				//�¼�����ѭ�������߳�ID one loop per thread

	Timestamp					m_tPollReturnTime;			//poll��ѯ���ص�ʱ��
	std::unique_ptr<EPoller>	m_pEPoller;					//��ǰ����ѭ��
	
	int							m_nWakupFd;					//���ڻ���IO�߳�	
	std::unique_ptr<Channel>	m_pWakeupChannel;			//���ڻ����¼���channel
	std::vector<Channel*>		m_vecActiveChannels;		//��ǰ�����Channels
	std::mutex					m_Mutex;					//�¼�����ѭ���Ļ�����
	std::vector<Functor>		m_vecPendingFunctors;		//�¼�����ѭ���������¼�
};