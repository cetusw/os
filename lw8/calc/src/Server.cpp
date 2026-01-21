#include "Server.h"
#include "Calculator.h"
#include "TCPSocket.h"
#include "ThreadPool.h"
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
			// TODO: shared
			++m_connectedClients;
			m_clientThreads.emplace_back([this, fd = std::move(clientFileDescriptor)]() mutable {
				HandleClient(std::move(fd));
				--m_connectedClients;
			});

			std::cout << "Active clients: " << m_connectedClients << std::endl;
		}
		catch (const std::exception& e)
		{
			std::cerr << "Accept error: " << e.what() << std::endl;
		}
	}
}

void Server::HandleClient(SocketHandler clientHandler)
{
	TCPSocket clientSocket(std::move(clientHandler));

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
}
