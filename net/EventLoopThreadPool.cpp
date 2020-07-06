#include "EventLoopThreadPool.h"

#include "EventLoop.h"
#include "EventLoopThread.h"

#include <functional>
#include <assert.h>

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop)
	: m_BaseLoop(baseLoop),
	  m_bStarted(false),
      m_nNumThreads(0),
	  m_nNext(0)
{
}

EventLoopThreadPool::~EventLoopThreadPool()
{
}

void EventLoopThreadPool::start()
{
	assert(!m_bStarted);
	m_BaseLoop->assertInLoopThread();

	m_bStarted = true;
	for (int i = 0; i < m_nNumThreads; ++i)
	{
		std::shared_ptr<EventLoopThread> t(new EventLoopThread);
		m_vecThreads.push_back(t);
		m_vecLoops.push_back(t->startLoop());
	}
}

EventLoop* EventLoopThreadPool::getNextLoop()
{
	m_BaseLoop->assertInLoopThread();
	EventLoop* loop = m_BaseLoop;
	if (!m_vecLoops.empty())
	{
		loop = m_vecLoops[m_nNext];
		++m_nNext;
		if (static_cast<size_t>(m_nNext) >= m_vecLoops.size())
			m_nNext = 0;
	}
	return loop;
}