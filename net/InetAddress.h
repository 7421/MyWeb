#pragma once

#include <string>
#include <netinet/in.h>

class InetAddress
{
public:
	explicit InetAddress(uint16_t port);

	//通过给定的IP地址和端口号构造 目的端地址
	InetAddress(const std::string& ip, uint16_t port);

	InetAddress(const struct sockaddr_in& addr)
		: m_sAddr(addr)
	{ }

	std::string toHostPort() const;

	const struct sockaddr_in& getSockAddrInet() const
	{
		return m_sAddr;
	}
	void setSockAddrInet(const struct sockaddr_in& addr) 
	{ 
		m_sAddr = addr;
	}
private:
	struct sockaddr_in m_sAddr;		//网络协议地址
};