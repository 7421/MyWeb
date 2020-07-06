#include "TcpServer.h"

#include <functional>
#include <sstream>

#include <assert.h>

#include "Acceptor.h"
#include "EventLoop.h"
#include "SocketsOps.h"
#include "TcpConnection.h"
#include "EventLoopThreadPool.h"
#include "../base/AsyncLog.h"


TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr)
	: m_Loop(loop),
	  m_strName(listenAddr.toHostPort()),
	  m_pAcceptor(new Acceptor(loop,listenAddr)),
	  m_pThreadPool(new EventLoopThreadPool(loop)),
	  m_bStarted(false),
	  m_nNextConnId(1)
{
	m_pAcceptor->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer()
{
}

void TcpServer::setThreadNum(int numThreads)
{
	assert(0 <= numThreads);
	m_pThreadPool->setThreadNum(numThreads);
}

void TcpServer::start()
{
	if (!m_bStarted)
	{
		m_bStarted = true;
		m_pThreadPool->start();
	}
	if (!m_pAcceptor->listenning())
		m_Loop->runInLoop(std::bind(&Acceptor::listen, &(*m_pAcceptor)));
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{
	m_Loop->assertInLoopThread();
	char buf[32];
	snprintf(buf, sizeof buf, "#%d", m_nNextConnId);
	++m_nNextConnId;
	std::string connName = m_strName + buf; //connName : 0.0.0.0:5001#1
	//TcpServer::newConnection [0.0.0.0:5001] - new connection [0.0.0.0:5001#1]
	//from 124.115.222.150:4147
	std::ostringstream os;
	os << "TcpServer::newConnection [" << m_strName
		<< "] - new connection [" << connName
		<< "] from " << peerAddr.toHostPort() << std::endl;
	LOGI(" %s ", os.str().c_str());
	InetAddress  localAddr(sockets::getLocalAddr(sockfd));

	EventLoop* ioLoop = m_pThreadPool->getNextLoop();
	TcpConnectionPtr conn(
		new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));
	m_mapConnections[connName] = conn;
	conn->setConnectionCallback(m_ConnectionCallback); //设置连接建立回调函数
	conn->setMessageCallback(m_MessageCallback);		  //设置消息到达回调函数
	conn->setWriteCompleteCallback(m_WriteCompleteCallback);
	conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1)); //设置连接关闭回调函数

	//回调连接建立函数
	ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}



void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
	m_Loop->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}


void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
	m_Loop->assertInLoopThread();
	LOGI(" TcpServer::removeConnection [ %s ] - coonection %s", m_strName.c_str(), conn->name().c_str());
	//返回值为1表示删除成功
	size_t n = m_mapConnections.erase(conn->name());
	assert(n == 1); (void)n;
	EventLoop* ioLoop = conn->getLoop();
	//通知IO线程，调用连接销毁程序
	ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}

