#pragma once


class InetAddress;

class Socket
{
public:
	explicit Socket(int sockfd)
		: m_nSockfd(sockfd)
	{
	}

	~Socket();

	Socket(const Socket&) = delete;
	void operator=(const Socket&) = delete;

	int fd() const
	{
		return m_nSockfd;
	}

	void bindAddress(const InetAddress& localaddr);

	void listen();

	int accept(InetAddress& peeraddr);

	void setReuseAddr(bool on);

	void shutdownWrite();

	void setTcpNoDelay(bool on);
private:
	const int m_nSockfd;	//Ì×½Ó×ÖÎÄ¼şÃèÊö·û
};