#pragma once

#include "Callbacks.h"
#include "TcpConnection.h"

#include <map>
#include <string>
#include <memory>

class Acceptor;
class EventLoop;
class EventLoopThreadPool;

class TcpServer
{
public:
	TcpServer(EventLoop* loop, const InetAddress& listenAddr);
	~TcpServer();

	TcpServer(const TcpServer&) = delete;
	void operator=(const TcpServer&) = delete;

	void setThreadNum(int numThreads);

	void start();

	void setConnectionCallback(const ConnectionCallback& cb)
	{
		m_ConnectionCallback = cb;
	}

	void setMessageCallback(const MessageCallback& cb)
	{
		m_MessageCallback = cb;
	}

	void setWriteCompleteCallback(const WriteCompleteCallback& cb)
	{
		m_WriteCompleteCallback = cb;
	}

private:
	void newConnection(int sockfd, const InetAddress& peerAddr);
	void removeConnection(const TcpConnectionPtr& conn);
	void removeConnectionInLoop(const TcpConnectionPtr& conn);

	typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

	EventLoop*								m_Loop;
	const std::string						m_strName;
	std::unique_ptr<Acceptor>				m_pAcceptor;
	std::unique_ptr<EventLoopThreadPool>	m_pThreadPool;
	ConnectionCallback						m_ConnectionCallback;
	WriteCompleteCallback					m_WriteCompleteCallback;
	MessageCallback							m_MessageCallback;
	bool									m_bStarted;
	int										m_nNextConnId;
	ConnectionMap							m_mapConnections;
};	