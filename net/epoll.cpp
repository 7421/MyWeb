#include "epoll.h"

#include "../base/AsyncLog.h"

#include <assert.h>
#include <sys/epoll.h>
#include <memory.h>
#include <unistd.h>
#include <errno.h>
#include "EventLoop.h"
#include "Channel.h"

//匿名空间，名称的作用域被限制在当前文件中
namespace
{
	const int kNew = -1;
	const int kAdded = 1;
	const int kDeleted = 2;
}
static const int  InitEventVectorSize = 16;

EPoller::EPoller(EventLoop* loop)
	: m_ownerLoop(loop),
	  m_nEpollfd(::epoll_create(EPOLL_CLOEXEC)),
	  m_vecEvents(InitEventVectorSize)
{
	if (m_nEpollfd < 0)
	{
		LOGE("Epoller::Epoller");
	}
}

EPoller::~EPoller()
{
	::close(m_nEpollfd);
}

Timestamp EPoller::poll(int timeoutMs, ChannelVector* activeChannels)
{
	int numEvents = ::epoll_wait(m_nEpollfd, &*m_vecEvents.begin(), static_cast<int>(m_vecEvents.size()), timeoutMs);
	Timestamp now(Timestamp::now());

	if (numEvents > 0)
	{
		LOGI("%d events happened", numEvents);
		fillActiveChannels(numEvents, activeChannels);
		if (static_cast<size_t>(numEvents) == m_vecEvents.size())
		{
			m_vecEvents.resize(m_vecEvents.size() * 2);
		}
	}
	else if (numEvents == 0)
	{
		LOGI(" nothing happended ");
	}
	else
	{
		if(errno != EINTR)
			LOGE(" EPoller::poll() Error %d ", errno);
	}
	return now;
}

void EPoller::fillActiveChannels(int numEvents, ChannelVector* activeChannels) const
{
	assert(static_cast<size_t>(numEvents) <= m_vecEvents.size());
	for (int i = 0; i < numEvents; ++i)
	{
		Channel* channel = static_cast<Channel*>(m_vecEvents[i].data.ptr);
		int fd = channel->fd();
		auto it = m_mapChannels.find(fd);
		assert(it != m_mapChannels.end());
		assert(it->second == channel);

		channel->set_revents(m_vecEvents[i].events);
		activeChannels->push_back(channel);
	}
}


void EPoller::updateChannel(Channel* channel)
{
	assertInLoopThread();
	LOGI("fd = %d events = %d ", channel->fd(), channel->events());
	const int state = channel->state();
	if (state == kNew || state == kDeleted)
	{
		// a new one, add with EPOLL_CTL_ADD
		int fd = channel->fd();
		if (state == kNew)
		{
			assert(m_mapChannels.find(fd) == m_mapChannels.end());
			m_mapChannels[fd] = channel;
		}
		else
		{
			assert(m_mapChannels.find(fd) != m_mapChannels.end());
			assert(m_mapChannels[fd] == channel);
		}
		channel->set_state(kAdded);
		update(EPOLL_CTL_ADD, channel);
	}
	else
	{
		int fd = channel->fd();
		(void)fd;
		assert(m_mapChannels.find(fd) != m_mapChannels.end());
		assert(m_mapChannels[fd] == channel);
		assert(state == kAdded);
		if (channel->isNoneEvent())
		{
			update(EPOLL_CTL_DEL, channel);
			channel->set_state(kDeleted);
		}
		else 
			update(EPOLL_CTL_MOD, channel);
	}
}

void EPoller::removeChannel(Channel* channel)
{
	assertInLoopThread();
	int fd = channel->fd();
	assert(m_mapChannels.find(fd) != m_mapChannels.end());
	assert(m_mapChannels[fd] == channel);
	assert(channel->isNoneEvent());

	int state = channel->state();
	assert(state == kAdded || state == kDeleted);
	size_t n = m_mapChannels.erase(fd);
	(void)n;
	assert(n == 1);
	if (state == kAdded)
		update(EPOLL_CTL_DEL, channel);
	channel->set_state(kNew);
}

void EPoller::update(int operation, Channel* channel)
{
	struct epoll_event event;
	bzero(&event, sizeof event);
	event.events = channel->events();
	event.data.ptr = channel;
	int fd = channel->fd();
	if (::epoll_ctl(m_nEpollfd, operation, fd, &event) < 0)
	{
		LOGE(" epoll_ctl op = %d fd = ", operation, fd);
	}
}