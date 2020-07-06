#pragma once

#include <string>
#include <netinet/in.h>

class InetAddress
{
public:
	explicit InetAddress(uint16_t port);

	//ͨ��������IP��ַ�Ͷ˿ںŹ��� Ŀ�Ķ˵�ַ
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
	struct sockaddr_in m_sAddr;		//����Э���ַ
};