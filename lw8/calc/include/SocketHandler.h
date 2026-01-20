#ifndef CALC_SOCKETHANDLER_H
#define CALC_SOCKETHANDLER_H

#include <unistd.h>

class SocketHandler
{
public:
	SocketHandler()
		: m_fileDescriptor(-1)
	{
	}

	explicit SocketHandler(const int fileDescriptor)
		: m_fileDescriptor(fileDescriptor)
	{
	}

	~SocketHandler()
	{
		Close();
	}

	SocketHandler(const SocketHandler&) = delete;

	SocketHandler& operator=(const SocketHandler&) = delete;

	SocketHandler(SocketHandler&& other) noexcept
		: m_fileDescriptor(other.m_fileDescriptor)
	{
		other.m_fileDescriptor = -1;
	}

	SocketHandler& operator=(SocketHandler&& other) noexcept
	{
		if (this != &other)
		{
			Close();
			m_fileDescriptor = other.m_fileDescriptor;
			other.m_fileDescriptor = -1;
		}
		return *this;
	}

	void Close()
	{
		if (IsValid())
		{
			close(m_fileDescriptor);
			m_fileDescriptor = -1;
		}
	}

	[[nodiscard]] int Get() const
	{
		return m_fileDescriptor;
	}

	[[nodiscard]] bool IsValid() const
	{
		return m_fileDescriptor != -1;
	}

private:
	int m_fileDescriptor;
};

#endif // CALC_SOCKETHANDLER_H
