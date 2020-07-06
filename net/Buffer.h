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

	//栈空间数据追加到缓冲区末尾
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
	//返回写入开始点（即数据结束点）
	char* beginWrite()
	{
		return begin() + m_unWriterIndex;
	}
	
	//将len个大小数据追加到可读数据头部
	void prepend(const void* data, size_t len)
	{
		assert(len <= prependableBytes());
		m_unReaderIndex -= len;
		const char* d = static_cast<const char*>(data);
		std::copy(d, d + len, begin() + m_unReaderIndex);
	}

	//直接读数据到buffer中
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
	多次从缓冲区独数据后，readerIndex会后移，导致预留变大
	在增大空间之前，先判断调整预留空间的大小后能否容纳要求的数据
	如果可以，则将预留空间缩小为8字节（默认的预留空间大小)
	如果不可以，那么就只能增加空间
	*/
	void makeSpace(size_t len);

	static const size_t m_unCheapPrepend = 8;	//默认预留字节数
	static const size_t m_unInitialSize = 1024; //默认不包括预留字节数的缓冲区大小

	std::vector<char>	m_vecBuffer;   //缓冲区
	size_t		m_unReaderIndex;	   //数据起始点
	size_t		m_unWriterIndex;	   //数据结束点
};