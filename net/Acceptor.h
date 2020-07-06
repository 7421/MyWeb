#pragma once

#include <functional>

#include "Channel.h"
#include "Socket.h"

class EventLoop;
class InetAddress;

class Acceptor
{
public:
	typedef std::function<void(int sockfd, const InetAddress&)> NewConnectionCallback;

	Acceptor(EventLoop* loop, const InetAddress& listenAddr);


	Acceptor(const Acceptor&) = delete;
	void operator=(const Acceptor&) = delete;

	void setNewConnectionCallback(const NewConnectionCallback& cb)
	{
		m_newConnectionCallback = cb;
	}

	bool listenning() const
	{
		return m_bListenning;
	}

	void listen();

private:
	void handleRead();

	EventLoop*				m_Loop;								//acceptor������eventloop
	Socket					m_acceptSocket;						//�������ӷ�����׽���
	Channel					m_acceptChannel;					//�������ӷ����channel����
	NewConnectionCallback	m_newConnectionCallback;			//���ӽ����¼�
	bool					m_bListenning;						//������ʶ
};