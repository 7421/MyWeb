
#include "./base/AsyncLog.h"
#include "./base/Timestamp.h"

#include "./net/EventLoop.h"

#include <unistd.h>

#include <iostream>
#include <thread>

using namespace std;

void threadFunc(EventLoop* loop)
{
	std::cout << "threadFunc(): pid = " << getpid()
		<< ", tid = " << std::this_thread::get_id() << std::endl;
	sleep(2);
	loop->quit();
}

int main()
{
	CAsyncLog::init();
	std::cout << "threadFunc(): pid = " << getpid()
		<< ", tid = " << std::this_thread::get_id() << std::endl;
	EventLoop  loop;
	std::thread	t(threadFunc, &loop);

	loop.loop();
	t.join();

	getchar();
}