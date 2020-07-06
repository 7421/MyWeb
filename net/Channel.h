#pragma once

#include <functional>
#include "../base/Timestamp.h"

class EventLoop;

class Channel
{
public:
	typedef std::function<void()>			EventCallback;
	typedef std::function<void(Timestamp)>	ReadEventCallback;

	Channel(EventLoop* loop, int fd);
	~Channel();

	void hanleEvent(Timestamp receiveTime);
	void setReadCallback(const ReadEventCallback& cb)
	{
		m_readCallback = cb;		//设置读回调函数
	}

	void setWriteCallback(const EventCallback& cb)
	{
		m_writeCallback = cb;	//设置写回调函数
	}

	void setErrorCallback(const EventCallback& cb)
	{
		m_errorCallback = cb;	//设置错误回调函数
	}

	void setCloseCallback(const EventCallback& cb)
	{
		m_closeCallback = cb;
	}

	int fd() const
	{
		return m_nFd;
	}

	int events() const
	{
		return m_nEvents;
	}

	void set_revents(int revt)
	{
		m_nRevents = revt;
	}

	bool isNoneEvent() const
	{
		return m_nNoneEvent == m_nEvents;
	}

	//使能读/写事件、禁止写事件，并通知当前文件描述符所属的loop更新轮询事件
	void enableReading() { m_nEvents  |= m_nReadEvent; update(); }
	void enableWriting() { m_nEvents  |= m_nWriteEvent; update(); }
	void disableWriting() { m_nEvents &= ~m_nWriteEvent; update(); }

	void disableAll() { m_nEvents = m_nNoneEvent; update(); }

	bool isWriting() const { return m_nEvents & m_nWriteEvent; }

	//for EPoller
	int state() { return m_nState; }
	void set_state(int state) { m_nState = state; }
	
	EventLoop* ownerLoop() { return m_pLoop; }
private:
	void update();

	static const int m_nNoneEvent;
	static const int m_nReadEvent;
	static const int m_nWriteEvent;


	EventLoop*		m_pLoop;				//Channel所属的事件驱动循环
	const int		m_nFd;					//每个Channel负责分发一个fd的事件
	int				m_nEvents;				//当前文件描述符关心的事件
	int				m_nRevents;				//目前活动的事件
	int				m_nState;				//used by Poller.

	bool			m_bEventHandling;		//正在运行事件处理标志

	ReadEventCallback	m_readCallback;		//读事件回调函数
	EventCallback		m_writeCallback;	//写事件回调函数
	EventCallback		m_errorCallback;	//错误事件回调函数
	EventCallback		m_closeCallback;	//关闭事件回调函数
};

