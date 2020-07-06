#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include <assert.h>

/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode

class Buffer
{
public:
	Buffer()
		: m_vecBuffer(m_unCheapPrepend + m_unInitialSize),
		m_unReaderIndex(m_unCheapPrepend),
		m_unWriterIndex(m_unCheapPrepend)
	{
		assert(readableBytes() == 0);
		assert(writableBytes() == m_unInitialSize);
		assert(prependableBytes() == m_unCheapPrepend);
	}

	void swap(Buffer& rhs)
	{
		m_vecBuffer.swap(rhs.m_vecBuffer);
		std::swap(m_unReaderIndex, rhs.m_unReaderIndex);
		std::swap(m_unWriterIndex, rhs.m_unWriterIndex);
	}

	size_t readableBytes() const
	{
		return m_unWriterIndex - m_unReaderIndex;
	}
	
	size_t writableBytes() const
	{
		return m_vecBuffer.size() - m_unWriterIndex;
	}
	
	size_t prependableBytes() const
	{
		return m_unReaderIndex;
	}

	const char* peek() const
	{
		return begin() + m_unReaderIndex;
	}

	void retrieve(size_t len)
	{
		assert(len <= readableBytes());
		m_unReaderIndex += len;
	}

	void retrieveUntil(const char* end)
	{
		assert(peek() <= end);
		assert(end <= beginWrite());
		retrieve(end - peek());
	}

	void retrieveAll()
	{
		m_unReaderIndex = m_unCheapPrepend;
		m_unWriterIndex = m_unCheapPrepend;
	}

	std::string retrieveAsString()
	{
		std::string str(peek(), readableBytes());
		retrieveAll();
		return str;
	}

	//ջ�ռ�����׷�ӵ�������ĩβ
	void append(const std::string& str)
	{
		append(str.data(), str.length());
	}
	void append(const void* data, size_t len)
	{
		append(static_cast<const char*>(data), len);
	}
	void append(const char* data, size_t len)
	{
		ensureWritableBytes(len);
		std::copy(data, data + len, beginWrite());
		m_unWriterIndex += len;
	}

	void ensureWritableBytes(size_t len)
	{
		if (writableBytes() < len)
			makeSpace(len);
		assert(writableBytes() >= len);
	}
	//����д�뿪ʼ�㣨�����ݽ����㣩
	char* beginWrite()
	{
		return begin() + m_unWriterIndex;
	}
	
	//��len����С����׷�ӵ��ɶ�����ͷ��
	void prepend(const void* data, size_t len)
	{
		assert(len <= prependableBytes());
		m_unReaderIndex -= len;
		const char* d = static_cast<const char*>(data);
		std::copy(d, d + len, begin() + m_unReaderIndex);
	}

	//ֱ�Ӷ����ݵ�buffer��
	ssize_t readFd(int fd, int* savedErrno);

private:
	char* begin()
	{
		return &*m_vecBuffer.begin();
	}

	const char* begin() const
	{
		return &*m_vecBuffer.begin();
	}
	/*
	��δӻ����������ݺ�readerIndex����ƣ�����Ԥ�����
	������ռ�֮ǰ�����жϵ���Ԥ���ռ�Ĵ�С���ܷ�����Ҫ�������
	������ԣ���Ԥ���ռ���СΪ8�ֽڣ�Ĭ�ϵ�Ԥ���ռ��С)
	��������ԣ���ô��ֻ�����ӿռ�
	*/
	void makeSpace(size_t len);

	static const size_t m_unCheapPrepend = 8;	//Ĭ��Ԥ���ֽ���
	static const size_t m_unInitialSize = 1024; //Ĭ�ϲ�����Ԥ���ֽ����Ļ�������С

	std::vector<char>	m_vecBuffer;   //������
	size_t		m_unReaderIndex;	   //������ʼ��
	size_t		m_unWriterIndex;	   //���ݽ�����
};