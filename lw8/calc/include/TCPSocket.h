#ifndef CALC_TCPSOCKET_H
#define CALC_TCPSOCKET_H

#include "SocketHandler.h"
#include <string>

class TCPSocket
{
public:
	TCPSocket();

	explicit TCPSocket(SocketHandler&& handle);

	void SendLine(const std::string& line) const;

	[[nodiscard]] std::string ReceiveLine() const;

	void Connect(const std::string& address, int port);

	void Bind(int port);

	void Listen() const;

	[[nodiscard]] SocketHandler Accept() const;

private:
	void CreateSocketHandler();

protected:
	SocketHandler m_handler;
	std::string m_buffer;
};

#endif // CALC_TCPSOCKET_H
