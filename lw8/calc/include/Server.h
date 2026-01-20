#ifndef CALC_SERVER_H
#define CALC_SERVER_H

#include "SocketHandler.h"
#include "TCPSocket.h"

#include <memory>
#include <thread>
#include <utility>
#include <vector>

class Server
{
public:
	void Start(int port);

private:
	struct ClientSession
	{
		std::jthread thread;
		std::shared_ptr<std::atomic<bool>> isFinished;

		ClientSession(std::jthread&& t, std::shared_ptr<std::atomic<bool>> f)
			: thread(std::move(t))
			, isFinished(std::move(f))
		{
		}
	};

	void CleanupFinishedThreads();
	static void HandleClient(SocketHandler clientHandler, const std::shared_ptr<std::atomic<bool>>& isFinished);
	std::vector<std::unique_ptr<ClientSession>> m_sessions;
};

#endif // CALC_SERVER_H
