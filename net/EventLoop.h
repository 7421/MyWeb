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
	
	//如果用户在当前IO线程调用这个函数，回调会同步进行
	//如果用户在其他线程调用runInLoop，cb会被加入队列IO线程会被唤醒来调用这个Functor
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
	
	bool						m_bQuit;					//停止标志，用于停止事件驱动循环EventLoop
	bool						m_bLooping;				//事件驱动循环EventLoop运行标志
	bool						m_bCallingPendingFunctors;  //正在处理事件标志
	const std::thread::id		m_spThreadId;				//事件驱动循环所属线程ID one loop per thread

	Timestamp					m_tPollReturnTime;			//poll轮询返回的时间
	std::unique_ptr<EPoller>	m_pEPoller;					//当前驱动循环
	
	int							m_nWakupFd;					//用于唤醒IO线程	
	std::unique_ptr<Channel>	m_pWakeupChannel;			//用于唤醒事件的channel
	std::vector<Channel*>		m_vecActiveChannels;		//当前激活的Channels
	std::mutex					m_Mutex;					//事件驱动循环的互斥锁
	std::vector<Functor>		m_vecPendingFunctors;		//事件驱动循环待处理事件
};