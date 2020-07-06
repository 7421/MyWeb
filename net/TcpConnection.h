#pragma once

#include "InetAddress.h"
#include "Buffer.h"
#include "Callbacks.h"

#include <memory>
#include <string>

class Channel;
class EventLoop;
class Socket;

class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
	TcpConnection(EventLoop* loop,
		const std::string& name,
		int sockfd,
		const InetAddress& localAddr,
		const InetAddress& peerAddr);
	~TcpConnection();
	
	TcpConnection(const TcpConnection&) = delete;
	void operator=(const TcpConnection&) = delete;
	
	EventLoop* getLoop() const { return m_Loop; }
	const std::string& name() const { return m_strName; }
	const InetAddress& localAddress() { return m_LocalAddr; }
	const InetAddress& peerAddress() { return m_PeerAddr; }
	bool connected() const { return m_emState == kConnected; }

	//Thread safe
	void send(const std::string& message);
	//Thread safe
	void shutdown();
	void setTcpNoDelay(bool on);

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

	void setCloseCallback(const CloseCallback& cb)
	{
		m_CloseCallback = cb;
	}
	//当tcpServer接受一个新的连接时 ，调用connectEstablished函数
	void connectEstablished();
	//当TcpServer将此TcpConnection移出ConnectionMap中时调用
	void connectDestroyed();

private:
	enum StateE { kConnecting, kConnected, kDisconnecting, kDisconnected, };

	void setState(StateE s) 
	{
		m_emState = s;
	}

	void handleRead(Timestamp receiveTime);
	void handleWrite();
	void handleClose();
	void handleError();

	void sendInLoop(const std::string& message);
	void shutdownInLoop();

	EventLoop*	m_Loop;
	std::string m_strName;
	StateE      m_emState;

	std::unique_ptr<Socket>		m_pSocket;		
	std::unique_ptr<Channel>	m_pChannel;		
	InetAddress					m_LocalAddr;
	InetAddress                 m_PeerAddr;

	ConnectionCallback			m_ConnectionCallback;
	MessageCallback				m_MessageCallback;
	WriteCompleteCallback		m_WriteCompleteCallback;
	CloseCallback				m_CloseCallback;
	Buffer						m_InputBuffer;
	Buffer						m_OutputBuffer;
};