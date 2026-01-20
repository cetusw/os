#ifndef CALC_CLIENT_H
#define CALC_CLIENT_H

#include "TCPSocket.h"

class Client
{
public:
	Client(std::string address, int port);
	void Run();

private:
	bool SendWithRetry(const std::string& command);
	std::string m_address;
	int m_port;
	TCPSocket m_socket;
};

#endif // CALC_CLIENT_H
