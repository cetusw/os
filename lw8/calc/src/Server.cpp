#include "Server.h"
#include "Calculator.h"
#include <iostream>

void Server::Start(const int port)
{
	TCPSocket listener;
	listener.Bind(port);
	listener.Listen();

	std::cout << "Server started on port " << port << std::endl;

	while (true)
	{
		try
		{
			SocketHandler clientFileDescriptor = listener.Accept();
			CleanupFinishedThreads();

			auto isFinished = std::make_shared<std::atomic<bool>>(false);

			std::jthread clientThread([clientFileDescriptor = std::move(clientFileDescriptor), isFinished]() mutable {
				HandleClient(std::move(clientFileDescriptor), isFinished);
			});

			m_sessions.push_back(std::make_unique<ClientSession>(std::move(clientThread), isFinished));

			std::cout << "Active clients: " << m_sessions.size() << std::endl;
		}
		catch (const std::exception& e)
		{
			std::cerr << "Accept error: " << e.what() << std::endl;
		}
	}
}

void Server::CleanupFinishedThreads()
{
	std::erase_if(m_sessions, [](const std::unique_ptr<ClientSession>& session) {
		return session->isFinished->load();
	});
}

void Server::HandleClient(SocketHandler clientHandler, const std::shared_ptr<std::atomic<bool>>& isFinished)
{
	const TCPSocket clientSocket(std::move(clientHandler));

	try
	{
		while (true)
		{
			std::string request = clientSocket.ReceiveLine();
			std::string response = Calculator::ProcessCommand(request);
			clientSocket.SendLine(response);
		}
	}
	catch (const std::exception& e)
	{
		std::cout << "Client disconnected: " << e.what() << std::endl;
	}

	isFinished->store(true);
}
