#include "Socket.h"

#include "InetAddress.h"
#include "SocketsOps.h"

#include <memory.h>
#include <netinet/tcp.h>


Socket::~Socket()
{
	sockets::close(m_nSockfd);
}
void Socket::bindAddress(const InetAddress& addr)
{
	sockets::bindOrDie(m_nSockfd, addr.getSockAddrInet());
}

int Socket::accept(InetAddress& peeraddr)
{
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof addr);
	int connfd = sockets::accept(m_nSockfd, &addr);
	if (connfd >= 0)
	{
		peeraddr.setSockAddrInet(addr);
	}
	return connfd;
}

void Socket::setReuseAddr(bool on)
{
	int optval = on ? 1 : 0;
	::setsockopt(m_nSockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
}

void Socket::shutdownWrite()
{
	sockets::shutdownWrite(m_nSockfd);
}

void Socket::setTcpNoDelay(bool on)
{
	int optval = on ? 1 : 0;
	::setsockopt(m_nSockfd, IPPROTO_TCP, TCP_NODELAY,
		&optval, sizeof optval);
	// FIXME CHECK
}

void Socket::listen()
{
	sockets::listenOrDie(m_nSockfd);
}