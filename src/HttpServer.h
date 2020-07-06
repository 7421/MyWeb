#pragma once
#include <memory>
#include "../net/Callbacks.h"
class TcpServer;
class EventLoop;
class InetAddress;

class HttpServer
{
public:
	HttpServer(EventLoop* loop, const InetAddress& listenAddr);
	~HttpServer();

	HttpServer(const HttpServer&) = delete;
	HttpServer& operator=(const HttpServer&) = delete;

	void setThreadNum(int n);
	void start();
private:
	void onConnection(const TcpConnectionPtr&);
	void onMessage(const TcpConnectionPtr&, Buffer* data, Timestamp);

	std::unique_ptr<TcpServer> m_TcpServer;
};