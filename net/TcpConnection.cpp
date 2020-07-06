#include "TcpConnection.h"

#include <functional>
#include <memory>
#include <sstream>

#include <unistd.h>
#include <assert.h>
#include <string.h>

#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"
#include "SocketsOps.h"

#include "../base/AsyncLog.h"

TcpConnection::TcpConnection(EventLoop* loop,
	const std::string& nameArg,
	int sockfd,
	const InetAddress& localAddr,
	const InetAddress& peerAddr)
	  : m_Loop(loop),
		m_strName(nameArg),
		m_emState(kConnecting),
		m_pSocket(new Socket(sockfd)),
		m_pChannel(new Channel(loop, sockfd)),
		m_LocalAddr(localAddr),
		m_PeerAddr(peerAddr)
{
	LOGI("TcpConnection::ctor[ %s ] at %p fd = %d", m_strName.c_str(), this, sockfd);
	m_pChannel->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
	m_pChannel->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
	m_pChannel->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
	m_pChannel->setErrorCallback(std::bind(&TcpConnection::handleError, this));
}

TcpConnection:: ~TcpConnection()
{
	LOGI("TcpConnection::dtor[ %s ] at %p fd = %d", m_strName.c_str(), this, m_pChannel->fd());
}

void TcpConnection::send(const std::string& message)
{
	if (m_emState == kConnected)
	{
		if (m_Loop->isInLoopThread())
			sendInLoop(message);
		else
			m_Loop->runInLoop(std::bind(&TcpConnection::sendInLoop, this, message));
	}
}
/*
	д������
	1.���Channelû�м�����д�¼������������Ϊ�գ�˵��֮ǰû�г����ں˻��������������ֱ��д���ں�
	2.���д���ں˳����Ҵ�����Ϣ��errno����EWOULDBLOCK��˵���ں˻�����������ʣ�ಿ����ӵ�Ӧ�ò����������
	3.���֮ǰ���������Ϊ�գ���ô��û�м����ں˻�����(fd)��д�¼�����ʼ����
*/
void TcpConnection::sendInLoop(const std::string& message)
{
	m_Loop->assertInLoopThread();
	//дtcp���������ֽ���
	ssize_t nwrote = 0;
	// if no thing in output queue, try writing directly
	// �����������������ݣ��Ͳ��ܳ��Է��������ˣ��������ݻ��ң�Ӧ��ֱ��д����������
	if (!m_pChannel->isWriting() && m_OutputBuffer.readableBytes() == 0)
	{
		nwrote = ::write(m_pChannel->fd(), message.data(), message.size());
		if (nwrote >= 0)
		{
			if (static_cast<size_t>(nwrote) < message.size())
				LOGI("I am going to write more data");
			else if (m_WriteCompleteCallback)
				m_Loop->queueInLoop(std::bind(m_WriteCompleteCallback, shared_from_this()));
		}
		else
		{
			nwrote = 0;
			if (errno != EWOULDBLOCK)
				LOGE(" TcpConnection::sendInLoop ");
		}
	}
	assert(nwrote >= 0);
	/* û����������һЩ����û��д��tcp�������У���ô����ӵ�д�������� */
	if (static_cast<size_t>(nwrote) < message.size())
	{
		m_OutputBuffer.append(message.data() + nwrote, message.size() - nwrote);
		/* ��û��д�������׷�ӵ�����������У�Ȼ�����Կ�д�¼��ļ��������֮ǰû���Ļ��� */
		if (!m_pChannel->isWriting())
			m_pChannel->enableWriting();
	}
}

void TcpConnection::shutdown()
{
	if (m_emState == kConnected)
	{
		setState(kDisconnecting);
		m_Loop->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
	}
}

void TcpConnection::shutdownInLoop()
{
	m_Loop->assertInLoopThread();
	//���û������д������ݣ��͹ر�
	if (!m_pChannel->isWriting())
		m_pSocket->shutdownWrite();
}

void TcpConnection::setTcpNoDelay(bool on)
{
	m_pSocket->setTcpNoDelay(on);
}

void TcpConnection::connectEstablished()
{
	m_Loop->assertInLoopThread();
	assert(m_emState == kConnecting);
	setState(kConnected);
	m_pChannel->enableReading();
	m_ConnectionCallback(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
	m_Loop->assertInLoopThread();
	assert(m_emState == kConnected || m_emState == kDisconnecting);
	setState(kDisconnected);
	m_pChannel->disableAll();
	m_ConnectionCallback(shared_from_this());
	m_Loop->removeChannel(&(*m_pChannel));
}

void TcpConnection::handleRead(Timestamp receiveTime)
{
	int savedErrno = 0;
	ssize_t n = m_InputBuffer.readFd(m_pChannel->fd(), &savedErrno);
	if (n > 0)
		m_MessageCallback(shared_from_this(), &m_InputBuffer, receiveTime);
	else if (n == 0)
		handleClose(); //0�ַ�����ʾ�ر�����
	else
	{
		errno = savedErrno;
		LOGE(" TcpConnection::handleRead ");
		handleError();
	}
}

// ��tcp��������дʱ���� 
void TcpConnection::handleWrite()
{
	m_Loop->assertInLoopThread();
	if (m_pChannel->isWriting())
	{
		//��д��д���������������ݣ�����ʵ��д����ֽ�����tcp���������п�����Ȼ���������������ݣ�
		ssize_t n = ::write(m_pChannel->fd(), m_OutputBuffer.peek(), m_OutputBuffer.readableBytes());
		if (n > 0)
		{
			//����д��������readerIndex
			m_OutputBuffer.retrieve(n);
			if (m_OutputBuffer.readableBytes() == 0)
			{
				m_pChannel->disableWriting();
				if (m_WriteCompleteCallback) {
					m_Loop->queueInLoop(
						std::bind(m_WriteCompleteCallback, shared_from_this()));
				}
				if (m_emState == kDisconnecting)
					shutdownInLoop();
			}
			else
				LOGI(" I am going to write more data");
		}
		else
		{
			LOGE(" TcpConnection::handleWrite ");
		}
	}
	else
		LOGI(" Connection is down, no more writing ");
}

void TcpConnection::handleClose()
{
	m_Loop->assertInLoopThread();
	LOGI(" TcpConnection::handleClose state = ");
	assert(m_emState == kConnected || m_emState == kDisconnecting);
	// we don't close fd, leave it to dtor, so we can find leaks easily.
	m_pChannel->disableAll();
	//must be the last line
	m_CloseCallback(shared_from_this());
}

__thread char t_errnobuf[512];
void TcpConnection::handleError()
{
	int err = sockets::getSocketError(m_pChannel->fd());
	std::ostringstream os;
	os << "TcpConnection::handleError [" << m_strName
		<< "] - SO_ERROR = " << err << " " << strerror(err) << std::endl;
	LOGE(" %s ", os.str().c_str());
}

