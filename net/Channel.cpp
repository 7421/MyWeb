#include "Channel.h"
#include "EventLoop.h"

#include "../base/Timestamp.h"
#include "../base/AsyncLog.h"

#include <poll.h>
#include <assert.h>

const int Channel::m_nNoneEvent =  0;
const int Channel::m_nReadEvent =  POLLIN | POLLPRI; //POLLIN(��ͨ���ݺ��������ݿɶ�)��POLLPRI�����������ݿɶ���
const int Channel::m_nWriteEvent = POLLOUT;			 //POLLOUT ��ͨ���ݿɶ�

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
	//֪ͨ�����¼���������Channel�б�
    m_pLoop->updateChannel(this);
}

void Channel::hanleEvent(Timestamp receiveTime)
{
    m_bEventHandling = true;
    //POLLNVAL: ָ�����ļ��������Ƿ�
    if (m_nRevents & POLLNVAL)
    {
        LOGI(" Channel::handle_event() POLLNVAL ");
    }
    //POLLRDHUP��TCP���ӱ��Զ˹رգ����߹ر���д����
    if ((m_nRevents & POLLHUP) && !(m_nRevents & POLLIN)) {
        LOGI("Channel::handle_event() POLLHUP");
        if (m_closeCallback) m_closeCallback();
    }
    //���������¼������ô���ص�����
    if (m_nRevents & (POLLERR | POLLNVAL))
        if (m_errorCallback) m_errorCallback();
    //����POLLIN����ͨ���¼������ȶ��¼�, POLLPRI: �����ȼ��ɶ���
    //POLLRDHUP��TCP���ӱ��Զ˹رգ����߹ر���д����
    if (m_nRevents & (POLLIN | POLLPRI | POLLRDHUP))
        if (m_readCallback) m_readCallback(receiveTime);
    //����POLLOUT: ��ͨ���ݿ�д�¼�
    if (m_nRevents & POLLOUT)
        if (m_writeCallback) m_writeCallback();
    m_bEventHandling = false;
}


