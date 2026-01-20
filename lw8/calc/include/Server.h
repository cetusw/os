#ifndef CALC_SERVER_H
#define CALC_SERVER_H

#include "SocketHandler.h"
#include "ThreadPool.h"
#include <memory>
#include <thread>

class Server
{
public:
	explicit Server(int maxConcurrentClients);
	void Start(int port);

private:
	static void HandleClient(SocketHandler clientHandler);
	ThreadPool m_clients;
	std::atomic<size_t> m_connectedClients{ 0 };
};

#endif // CALC_SERVER_H
