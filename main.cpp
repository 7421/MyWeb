
#include "./base/AsyncLog.h"

#include "./net/EventLoop.h"
#include "./net/InetAddress.h"
#include "./src/HttpServer.h"

int main(int argc,char *argv[])
{
	//CAsyncLog::init();
	if (argc > 1)
	{
		InetAddress  addr(5001);
		EventLoop	loop;
		HttpServer  httpserver(&loop, addr);
		httpserver.setThreadNum(atoi(argv[1]));
		httpserver.start();
		loop.loop();
	}
	else
		LOGI("Usage : the threads number error");
	//CAsyncLog::uninit();
	return 0;
}
