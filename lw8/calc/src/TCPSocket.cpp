#include "TCPSocket.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>

// TODO: длинные выражения

TCPSocket::TCPSocket() = default;

TCPSocket::TCPSocket(SocketHandler&& handle)
	: m_handler(std::move(handle))
{
}

void TCPSocket::Connect(const std::string& address, const int port)
{
	CreateSocketHandler();
	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	if (inet_pton(AF_INET, address.c_str(), &addr.sin_addr) <= 0)
	{
		throw std::runtime_error("Invalid address format");
	}

	if (connect(m_handler.Get(), reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
	{
		throw std::runtime_error("Connect failed: " + std::string(strerror(errno)));
	}
}

std::string TCPSocket::ReceiveLine()
{
	while (true)
	{
		const size_t endOfLinePos = m_buffer.find('\n');
		if (endOfLinePos != std::string::npos)
		{
			std::string line = m_buffer.substr(0, endOfLinePos);
			m_buffer.erase(0, endOfLinePos + 1);
			return line;
		}

		char temp[4096];
		const ssize_t bytesRead = recv(m_handler.Get(), temp, sizeof(temp), 0);

		if (bytesRead <= 0)
		{
			throw std::runtime_error("Connection closed or error");
		}

		m_buffer.append(temp, bytesRead);
	}
}

void TCPSocket::SendLine(const std::string& line) const
{
	const std::string data = line + "\n";
	const ssize_t result = send(m_handler.Get(), data.c_str(), data.size(), 0);
	if (result < 0)
	{
		throw std::runtime_error("Send failed");
	}
}

void TCPSocket::Bind(const int port)
{
	if (!m_handler.IsValid())
	{
		CreateSocketHandler();
	}

	constexpr int opt = 1;
	if (setsockopt(m_handler.Get(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		throw std::runtime_error("Failed to set SO_REUSEADDR: " + std::string(strerror(errno)));
	}

	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	if (bind(m_handler.Get(), reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
	{
		throw std::runtime_error("Bind failed: " + std::string(strerror(errno)));
	}
}

void TCPSocket::Listen() const
{
	if (listen(m_handler.Get(), SOMAXCONN) < 0)
	{
		throw std::runtime_error("Listen failed: " + std::string(strerror(errno)));
	}
}

SocketHandler TCPSocket::Accept() const
{
	const int clientFileDescriptor = accept(m_handler.Get(), nullptr, nullptr);

	if (clientFileDescriptor < 0)
	{
		throw std::runtime_error("Accept failed: " + std::string(strerror(errno)));
	}

	return SocketHandler(clientFileDescriptor);
}

void TCPSocket::CreateSocketHandler()
{
	const int fileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
	if (fileDescriptor == -1)
	{
		throw std::runtime_error("Failed to create socket for binding");
	}
	m_handler = SocketHandler(fileDescriptor);
}