#include "InetAddress.h"

#include <string>
#include <netinet/in.h>
#include <memory.h>
#include <arpa/inet.h>


#include "../base/AsyncLog.h"
/*
	struct sockaddr_in{
		sa_family_t		sin_family;
		uint16_t		sin_port;
		struct in_addr  sin_addr;
	};

  //Internet address
  typedef uint32_t in_addr_t;
  struct in_addr{
		in_addr_t	s_addr;
	};
*/

static const in_addr_t kInaddrAny = INADDR_ANY;

InetAddress::InetAddress(uint16_t port)
{
	memset(&m_sAddr, 0, sizeof m_sAddr); //addr_结构体初始化为0
	m_sAddr.sin_family = AF_INET;
	m_sAddr.sin_addr.s_addr = htonl(kInaddrAny);
	m_sAddr.sin_port = htons(port);
}

InetAddress::InetAddress(const std::string& ip, uint16_t port)
{
	memset(&m_sAddr, 0, sizeof m_sAddr);
	m_sAddr.sin_family = AF_INET;
	m_sAddr.sin_port = htons(port);
	if (::inet_pton(AF_INET, ip.c_str(), &m_sAddr.sin_addr) <= 0)
	{
		LOGE(" sockets::fromHostPort ");
	}
}

std::string InetAddress::toHostPort() const
{
	char buf[32];
	char host[INET_ADDRSTRLEN] = "INVALID";
	::inet_ntop(AF_INET, &m_sAddr.sin_addr, host, sizeof host);
	uint16_t port = ntohs(m_sAddr.sin_port);
	snprintf(buf, 32, "%s:%u", host, port);
	return buf;
}
