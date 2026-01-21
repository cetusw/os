#ifndef CALC_SERVER_H
#define CALC_SERVER_H

#include "SocketHandler.h"
#include <memory>
#include <thread>
#include <vector>

class Server
{
public:
	void Start(int port);

private:
	static void HandleClient(SocketHandler clientHandler);

	std::vector<std::jthread> m_clientThreads;
	std::atomic<size_t> m_connectedClients{ 0 };
};

#endif // CALC_SERVER_H
