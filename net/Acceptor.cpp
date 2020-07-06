#include "Acceptor.h"

#include <functional>

#include "EventLoop.h"
#include "InetAddress.h"
#include "SocketsOps.h"

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr)
	: m_Loop(loop),
	  m_acceptSocket(sockets::createNonblockingOrDie()),//创建服务器端非阻塞套接字Socket
	  m_acceptChannel(loop,m_acceptSocket.fd()),
	  m_bListenning(false)
{
	m_acceptSocket.setReuseAddr(true);
	m_acceptSocket.bindAddress(listenAddr);
	m_acceptChannel.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

void Acceptor::listen()
{
	m_Loop->assertInLoopThread();
	m_bListenning = true;
	m_acceptSocket.listen();
	m_acceptChannel.enableReading();
}

void Acceptor::handleRead()
{
	m_Loop->assertInLoopThread();
	InetAddress peerAddr(0);
	int connfd = m_acceptSocket.accept(peerAddr);
	if (connfd >= 0)
	{
		if (m_newConnectionCallback)
			m_newConnectionCallback(connfd, peerAddr);
		else
			sockets::close(connfd);
	}
}