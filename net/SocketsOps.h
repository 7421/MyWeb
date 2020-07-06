#pragma once

#include <arpa/inet.h>
#include <endian.h>

namespace sockets
{
	//主机存储次序转网络存储次序
	inline uint64_t hostToNetwork64(uint64_t host64)
	{
		return htobe64(host64);
	}
	inline uint32_t hostToNetwork32(uint32_t host32)
	{
		return htonl(host32);
	}
	inline uint16_t hostToNetwork16(uint16_t host16)
	{
		return htons(host16);
	}
	//网络存储次序转主机存储次序
	inline uint64_t networkToHost64(uint64_t net64)
	{
		return be64toh(net64);
	}

	inline uint32_t networkToHost32(uint32_t net32)
	{
		return ntohl(net32);
	}
	inline uint16_t networkToHost16(uint16_t net16)
	{
		return ntohs(net16);
	}
	//创建一个非阻塞套接字文件描述符
	int createNonblockingOrDie();

	int  connect(int sockfd, const struct sockaddr_in& addr);
	void bindOrDie(int sockfd, const struct sockaddr_in& addr);
	void listenOrDie(int sockfd);
	int  accept(int sockfd, struct sockaddr_in* addr);
	void close(int sockfd);
	void shutdownWrite(int sockfd);

	void toHostPort(char* buf, size_t size,
		const struct sockaddr_in& addr);
	void fromHostPort(const char* ip, uint16_t port,
		struct sockaddr_in* addr);
	//获得sockfd套接字捆绑的本地协议地址
	struct sockaddr_in getLocalAddr(int sockfd);
	struct sockaddr_in getPeerAddr(int sockfd);

	int getSocketError(int sockfd);
	bool isSelfConnect(int sockfd);
}