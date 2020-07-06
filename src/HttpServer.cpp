#include "HttpServer.h"
#include "../net/TcpServer.h"
#include "../net/TcpConnection.h"
#include "../base/AsyncLog.h"

#include <functional>
#include <fstream>
#include <sstream>
HttpServer::HttpServer(EventLoop* loop, const InetAddress& listenAddr)
	: m_TcpServer(new TcpServer(loop,listenAddr))
{
	m_TcpServer->setConnectionCallback(std::bind(&HttpServer::onConnection,this,std::placeholders::_1));
	m_TcpServer->setMessageCallback(std::bind(&HttpServer::onMessage,this,std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

HttpServer::~HttpServer()
{
}

void HttpServer::onConnection(const TcpConnectionPtr& conn)
{

}

void HttpServer::setThreadNum(int n)
{
    assert(0 <= n);
    m_TcpServer->setThreadNum(n);
}

void HttpServer::start()
{
    m_TcpServer->start();
}

void HttpServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp timestamp)
{
	LOGI("%s", buf->retrieveAsString().c_str());
    std::string     toSend = "HTTP/1.1 200 ok\r\nconnection: close\r\n\r\n";
    std::fstream f;
    f.open("../../Web.html");
    std::stringstream ss;
    ss << f.rdbuf();
    toSend += ss.str();
    conn->send(toSend);
    conn->shutdown();
}

