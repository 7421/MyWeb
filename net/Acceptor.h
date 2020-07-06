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

	EventLoop*				m_Loop;								//acceptor所属的eventloop
	Socket					m_acceptSocket;						//接收连接服务端套接字
	Channel					m_acceptChannel;					//接收连接服务端channel渠道
	NewConnectionCallback	m_newConnectionCallback;			//连接建立事件
	bool					m_bListenning;						//监听标识
};