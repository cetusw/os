#include "Client.h"
#include <chrono>
#include <iostream>
#include <utility>

Client::Client(std::string address, const int port)
	: m_address(std::move(address))
	, m_port(port)
{
}

void Client::Run()
{
	try
	{
		m_socket.Connect(m_address, m_port);
		std::cout << "Connected to server at " << m_address << ":" << m_port << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Initial connection failed: " << e.what() << std::endl;
	}

	std::string input;
	while (std::getline(std::cin, input))
	{
		if (input.empty())
		{
			continue;
		}
		if (SendWithRetry(input))
		{
			try
			{
				std::cout << "Result: " << m_socket.ReceiveLine() << std::endl;
			}
			catch (...)
			{
				std::cout << "Error: Lost connection during receive" << std::endl;
			}
		}
	}
}

bool Client::SendWithRetry(const std::string& command)
{
	int attempts = 3;
	while (attempts > 0)
	{
		try
		{
			m_socket.SendLine(command);
			return true;
		}
		catch (...)
		{
			attempts--;
			std::cout << "Connection lost. Retrying... (" << attempts << " left)" << std::endl;
			try
			{
				m_socket.Connect(m_address, m_port);
				std::cout << "Reconnected successfully!" << std::endl;
			}
			catch (...)
			{
				std::cout << "Still cannot connect to server..." << std::endl;
			}
		}
	}
	std::cout << "Failed to connect after multiple attempts." << std::endl;
	return false;
}
