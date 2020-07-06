#include "Channel.h"
#include "EventLoop.h"

#include "../base/Timestamp.h"
#include "../base/AsyncLog.h"

#include <poll.h>
#include <assert.h>

const int Channel::m_nNoneEvent =  0;
const int Channel::m_nReadEvent =  POLLIN | POLLPRI; //POLLIN(普通数据和优先数据可读)，POLLPRI（高优先数据可读）
const int Channel::m_nWriteEvent = POLLOUT;			 //POLLOUT 普通数据可读

Channel::Channel(EventLoop* loop, int fdArg)
	: m_pLoop(loop),
	  m_nFd(fdArg),
	  m_nEvents(0),
	  m_nRevents(0),
	  m_nState(-1),
	  m_bEventHandling(false)
{

}

Channel::~Channel()
{
	assert(!m_bEventHandling);
}

void Channel::update()
{
	//通知所属事件环，更新Channel列表
    m_pLoop->updateChannel(this);
}

void Channel::hanleEvent(Timestamp receiveTime)
{
    m_bEventHandling = true;
    //POLLNVAL: 指定的文件描述符非法
    if (m_nRevents & POLLNVAL)
    {
        LOGI(" Channel::handle_event() POLLNVAL ");
    }
    //POLLRDHUP：TCP连接被对端关闭，或者关闭了写操作
    if ((m_nRevents & POLLHUP) && !(m_nRevents & POLLIN)) {
        LOGI("Channel::handle_event() POLLHUP");
        if (m_closeCallback) m_closeCallback();
    }
    //发生错误事件，调用错误回调函数
    if (m_nRevents & (POLLERR | POLLNVAL))
        if (m_errorCallback) m_errorCallback();
    //发生POLLIN：普通读事件、优先读事件, POLLPRI: 高优先级可读，
    //POLLRDHUP：TCP连接被对端关闭，或者关闭了写操作
    if (m_nRevents & (POLLIN | POLLPRI | POLLRDHUP))
        if (m_readCallback) m_readCallback(receiveTime);
    //发生POLLOUT: 普通数据可写事件
    if (m_nRevents & POLLOUT)
        if (m_writeCallback) m_writeCallback();
    m_bEventHandling = false;
}


