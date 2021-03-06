#include <string>

#include "SocketsOps.h"
#include "../base/AsyncLog.h"


#include <fcntl.h>
#include <memory.h>
#include <sys/socket.h>
#include <unistd.h>

//匿名命名空间，名称的作用域被限制在当前文件中，无法通过在另外的文件中使用extern声明来进行链接
namespace
{
	typedef struct sockaddr SA;
	//sockaddr_in 转sockaddr函数
	const SA* sockaddr_cast(const struct sockaddr_in* addr)
	{
		return reinterpret_cast<const SA*>(addr);
	}

	SA* sockaddr_cast(struct sockaddr_in* addr)
	{
		return reinterpret_cast<SA*>(addr);
	}

}
//创建网络通信套接字
//成功返回文件描述符；失败产生异常，程序退出
int sockets::createNonblockingOrDie()
{
	//创建网络通信套接字，IPv4协议族，面向连接传输方式，TCP协议，非阻塞，exe函数运行其他程序时自动关闭该文件描述符
	int	sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);

	if (sockfd < 0)
	{
		LOGE(" sockets::createNonblockingOrDie ");
	}
	return sockfd;
}

int sockets::connect(int sockfd, const struct sockaddr_in& addr)
{
	return ::connect(sockfd, sockaddr_cast(&addr), sizeof addr);
}

//将一个本地协议地址赋给套接字sockfd，失败产生异常，程序退出
void sockets::bindOrDie(int sockfd, const struct sockaddr_in& addr)
{
	int ret = ::bind(sockfd, sockaddr_cast(&addr), sizeof addr);
	if (ret < 0)
	{
		LOGE("sockets::bindOrDie");
	}
}
//1.将文件描述符转为被动态
//2.规定内核为该套接字设置的排队的最大连接数
void sockets::listenOrDie(int sockfd)
{
	int ret = ::listen(sockfd, SOMAXCONN);
	if (ret < 0)
	{
		LOGE(" sockets::listenOrDie ");
	}
}

int sockets::accept(int sockfd, struct sockaddr_in* addr)
{
	socklen_t addrlen = sizeof * addr;

	int connfd = ::accept4(sockfd, sockaddr_cast(addr),
		&addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
	if (connfd < 0)
	{
		LOGE(" Socket::accept ");
	}
	return connfd;
}

void sockets::close(int sockfd)
{
	if (::close(sockfd) < 0)
	{
		LOGE(" sockets::close ");
	}
}

void sockets::shutdownWrite(int sockfd)
{
	if (::shutdown(sockfd, SHUT_WR) < 0)
	{
		LOGE(" sockets::shutdown Write ");
	}
}

//根据网络序协议地址转为主机序协议地址
//已buf字符串的形式表达
void sockets::toHostPort(char* buf, size_t size, const struct sockaddr_in& addr)
{
	char host[INET_ADDRSTRLEN] = "INVALID"; //INET_ADDRSTRLEN 16
	::inet_ntop(AF_INET, &addr.sin_addr, host, sizeof host);
	uint16_t port = sockets::networkToHost16(addr.sin_port);
	snprintf(buf, size, "%s:%u", host, port);
}
//根据IP地址，和port端口号,产生本地协议地址
void sockets::fromHostPort(const char* ip, uint16_t port, struct sockaddr_in* addr)
{
	addr->sin_family = AF_INET;
	addr->sin_port = hostToNetwork16(port);
	if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
	{
		LOGE(" sockets::fromHostPort ");
	}
}
//获得sockfd套接字捆绑的本地协议地址
struct sockaddr_in sockets::getLocalAddr(int sockfd)
{
	struct sockaddr_in localaddr;
	bzero(&localaddr, sizeof localaddr);
	socklen_t addrlen = sizeof(localaddr);
	if (::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0)
	{
		LOGE(" sockets::getLocalAddr ");
	}
	return localaddr;
}

struct sockaddr_in sockets::getPeerAddr(int sockfd)
{
	struct sockaddr_in peeraddr;
	bzero(&peeraddr, sizeof peeraddr);
	socklen_t addrlen = sizeof(peeraddr);
	if (::getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen) < 0)
	{
		LOGE(" sockets::getPeerAddr ");
	}
	return peeraddr;
}


int sockets::getSocketError(int sockfd)
{
	int optval;
	socklen_t optlen = sizeof optval;

	if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
	{
		return errno;
	}
	else
	{
		return optval;
	}
}

bool sockets::isSelfConnect(int sockfd)
{
	struct sockaddr_in localaddr = getLocalAddr(sockfd);
	struct sockaddr_in peeraddr = getPeerAddr(sockfd);
	return localaddr.sin_port == peeraddr.sin_port
		&& localaddr.sin_addr.s_addr == peeraddr.sin_addr.s_addr;
}


