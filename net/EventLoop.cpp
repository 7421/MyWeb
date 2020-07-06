#include "EventLoop.h"

#include <thread>
#include <sstream>
#include <functional>

#include <unistd.h>
#include <assert.h>
#include <sys/eventfd.h>

#include "../base/AsyncLog.h"
#include "../base/Timestamp.h"

#include "epoll.h"
#include "Channel.h"

__thread EventLoop* t_loopInThisThread = 0;
const int nPollTimeMs = 10000;

EventLoop::EventLoop()
	:	m_bLooping(false),
		m_bQuit(false),
		m_bCallingPendingFunctors(false),
		m_spThreadId(std::this_thread::get_id()),
		m_pEPoller(new EPoller(this)),
		m_nWakupFd(createEventfd()),
	    m_pWakeupChannel(new Channel(this,m_nWakupFd))
{
	std::ostringstream osThreadId;
	osThreadId << std::this_thread::get_id();
	LOGI("EventLoop created %p in thread % s", this, osThreadId.str().c_str());

	osThreadId.clear();
	if (t_loopInThisThread)
	{
		//同一个线程出现了两个EventLoop
		osThreadId << "Another EventLoop " << t_loopInThisThread
			<< " exists in this thread " << m_spThreadId;
		LOGE(" %s ", osThreadId.str().c_str());
	}
	else
		t_loopInThisThread = this;
	m_pWakeupChannel->setReadCallback(std::bind(&EventLoop::handleRead, this));
	m_pWakeupChannel->enableReading();
}

void EventLoop::loop()
{
	assert(!m_bLooping);
	m_bLooping = true;
	assertInLoopThread();
	m_bLooping = true;
	m_bQuit = false;
	while (!m_bQuit)
	{
		m_vecActiveChannels.clear();
		m_tPollReturnTime = m_pEPoller->poll(nPollTimeMs, &m_vecActiveChannels);
		for (auto it = m_vecActiveChannels.begin(); it != m_vecActiveChannels.end(); ++it)
			(*it)->hanleEvent(m_tPollReturnTime);
		doPendingFunctors();
	}
	LOGI("EventLoop %p stop looping ", this);
	m_bLooping = false;
}

EventLoop::~EventLoop()
{
	assert(!m_bLooping);
	//析构函数，将该线程所有的EventLoop*设为空
	::close(m_nWakupFd);
	t_loopInThisThread = NULL;
}
//在loop的IO线程内执行某个用户任务回调
void EventLoop::runInLoop(const Functor& cb)
{
	if (isInLoopThread())
		cb();
	else
		queueInLoop(cb);
}

void EventLoop::queueInLoop(const Functor& cb)
{
	//由于pendingFunctor会暴露给其他线程，先上锁
	{
		std::lock_guard<std::mutex>	    lock(m_Mutex);
		m_vecPendingFunctors.push_back(cb);
	}

	//如果当前线程不是IO线程，则唤醒IO线程处理事件
	if (!isInLoopThread() || m_bCallingPendingFunctors)
		wakeup();
}

void EventLoop::updateChannel(Channel* channel)
{
	assert(channel->ownerLoop() == this);
	assertInLoopThread();

	m_pEPoller->updateChannel(channel); //通知poller轮循器更新channellist_列表
}

void EventLoop::removeChannel(Channel* channel)
{
	assert(channel->ownerLoop() == this);
	assertInLoopThread();
	m_pEPoller->removeChannel(channel);
}

void EventLoop::assertInLoopThread()
{
	if (!isInLoopThread())
	{
		std::ostringstream os;
		os << "EventLoop::abortNotInLoopThread - EventLoop " << this
			<< " was created in threadId_ " << m_spThreadId
			<< ", current thread id = " << std::this_thread::get_id();
		LOGE(" %s ", os.str().c_str());
	}
}

void EventLoop::wakeup()
{
	uint64_t one = 1;
	ssize_t n = ::write(m_nWakupFd, &one, sizeof one);
	if (n != sizeof one)
	{
		LOGE("EventLoop::wakeup() writes %d instead of 8", n);
	}
}

void EventLoop::handleRead()
{
	uint64_t one = 1;
	ssize_t n = ::read(m_nWakupFd, &one, sizeof one);
	if (n != sizeof one)
		LOGE("EventLoop::wakeup() writes %d instead of 8", n);
}


void EventLoop::doPendingFunctors()
{
	std::vector<Functor> functors;
	m_bCallingPendingFunctors = true;
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		functors.swap(m_vecPendingFunctors); //减少临界区复杂度，一次性将回调队列换出
	}
	//逐个执行用户回调函数
	for (size_t i = 0; i < functors.size(); ++i)
	{
		functors[i]();
	}
	m_bCallingPendingFunctors = false;
}

int EventLoop::createEventfd()
{
	//内置的64位计数器初始值为0
	//EFD_NONBLOCK : 非阻塞模式
	//EFD_CLOEXEC  : 表示返回的eventfd文件描述符
	//用exec函数调用其他程序时会自动关闭这个文件描述符
	int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if (evtfd < 0)
	{
		LOGE(" Failed in eventfd ");
	}
	return evtfd;
}

void EventLoop::quit()
{
	m_bQuit = true;
	if (!isInLoopThread()) //不是loop所属线程，利用eventfd唤醒IO线程
		wakeup();
}


