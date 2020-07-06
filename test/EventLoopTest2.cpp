
#include "./base/AsyncLog.h"
#include "./base/Timestamp.h"

#include "./net/EventLoop.h"

#include <unistd.h>

#include <iostream>
#include <thread>

using namespace std;

EventLoop* gloop;
void print()
{
	LOGI("hello world!");
}
void threadFunc()
{
	std::cout << "threadFunc(): pid = " << getpid()
		<< ", tid = " << std::this_thread::get_id() << std::endl;
	sleep(2);
	gloop->runInLoop(print);
}



int main()
{
	EventLoop loop;
	CAsyncLog::init();
	std::cout << "threadFunc(): pid = " << getpid()
		<< ", tid = " << std::this_thread::get_id() << std::endl;
	gloop = &loop;
	std::thread	t(threadFunc);

	loop.loop();
	getchar();
}