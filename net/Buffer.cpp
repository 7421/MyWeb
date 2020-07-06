#include "Buffer.h"

#include <errno.h>
#include <memory.h>
#include <sys/uio.h>

ssize_t Buffer::readFd(int fd, int* savedErrno)
{
	char extrabuf[65536];
	struct iovec vec[2];
	const size_t writable = writableBytes();

	vec[0].iov_base = begin() + m_unWriterIndex;
	vec[0].iov_len = writable;
	vec[1].iov_base = extrabuf;
	vec[1].iov_len = sizeof extrabuf;

	const size_t n = readv(fd, vec, 2);

	if (n < 0)
		*savedErrno = errno;
	else if (static_cast<size_t>(n) <= writable)
		m_unWriterIndex += n;
	else
	{
		m_unWriterIndex = m_vecBuffer.size();
		append(extrabuf, n - writable);
	}
	return n;
}

void Buffer::makeSpace(size_t len)
{
	if (writableBytes() + prependableBytes() < len + m_unCheapPrepend)
		m_vecBuffer.resize(m_unWriterIndex + len);
	else
	{
		//move readable data to the front, make space inside buffer
		assert(m_unCheapPrepend < m_unReaderIndex);
		size_t readable = readableBytes();
		std::copy(begin() + m_unReaderIndex, begin() + m_unWriterIndex,
				 begin() + m_unCheapPrepend);
		m_unReaderIndex = m_unCheapPrepend;
		m_unWriterIndex = m_unReaderIndex + readable;
		assert(readable == readableBytes());
	}
}