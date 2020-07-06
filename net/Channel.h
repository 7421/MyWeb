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
		m_readCallback = cb;		//���ö��ص�����
	}

	void setWriteCallback(const EventCallback& cb)
	{
		m_writeCallback = cb;	//����д�ص�����
	}

	void setErrorCallback(const EventCallback& cb)
	{
		m_errorCallback = cb;	//���ô���ص�����
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

	//ʹ�ܶ�/д�¼�����ֹд�¼�����֪ͨ��ǰ�ļ�������������loop������ѯ�¼�
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


	EventLoop*		m_pLoop;				//Channel�������¼�����ѭ��
	const int		m_nFd;					//ÿ��Channel����ַ�һ��fd���¼�
	int				m_nEvents;				//��ǰ�ļ����������ĵ��¼�
	int				m_nRevents;				//Ŀǰ����¼�
	int				m_nState;				//used by Poller.

	bool			m_bEventHandling;		//���������¼������־

	ReadEventCallback	m_readCallback;		//���¼��ص�����
	EventCallback		m_writeCallback;	//д�¼��ص�����
	EventCallback		m_errorCallback;	//�����¼��ص�����
	EventCallback		m_closeCallback;	//�ر��¼��ص�����
};

