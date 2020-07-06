#pragma once

#include <vector>
#include <map>

#include "../base/Timestamp.h"
#include "EventLoop.h"

class Channel;

class EPoller
{
public:
	typedef std::vector<Channel*> ChannelVector;
	
	EPoller(EventLoop* loop);
	~EPoller();

	EPoller(const EPoller&) = delete;
	void operator=(const EPoller&) = delete;

	Timestamp poll(int timeoutMs, ChannelVector* activeChannels);

	void updateChannel(Channel* channel);
	void removeChannel(Channel* channel);

	void assertInLoopThread() { m_ownerLoop->assertInLoopThread(); }
private: 


	void fillActiveChannels(int numEvents, ChannelVector* activeChannels) const;

	void update(int operation, Channel* channel);

	EventLoop*						m_ownerLoop;	//��ǰ��ѯ���������¼�����ѭ��
	int								m_nEpollfd;		//Epollfd�ļ�����������epoll_create����
	std::vector<struct epoll_event> m_vecEvents;	//epoll_event����
	std::map<int, Channel*>			m_mapChannels;  //Epollfd��������Channel��fd -> Channelӳ��
};