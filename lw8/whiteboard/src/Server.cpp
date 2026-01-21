#include "Server.h"
#include <iostream>

Server::Server(const short port)
	: m_acceptor(m_ioContext, tcp::endpoint(tcp::v4(), port))
{
}

Server::~Server()
{
	m_ioContext.stop();
}

void Server::Start()
{
	std::cout << "Server started" << std::endl;

	Accept();

	m_networkThread = std::jthread([this]() {
		m_ioContext.run();
	});
}

void Server::Accept()
{
	auto socket = std::make_shared<tcp::socket>(m_ioContext);

	m_acceptor.async_accept(*socket, [this, socket](const boost::system::error_code& error) {
		if (!error)
		{
			{
				std::lock_guard lock(m_mutex);
				m_clients.push_back(socket);

				for (const auto& data : m_history)
				{
					boost::asio::async_write(*socket, boost::asio::buffer(&data, sizeof(DrawData)),
						[](const boost::system::error_code&, std::size_t) {});
				}
			}
			HandleClient(socket);
		}
		Accept();
	});
}

void Server::HandleClient(const std::shared_ptr<tcp::socket>& socket)
{
	auto buffer = std::make_shared<DrawData>();

	boost::asio::async_read(*socket, boost::asio::buffer(buffer.get(), sizeof(DrawData)),
		[this, socket, buffer](const boost::system::error_code& error, std::size_t) {
			if (!error)
			{
				{
					std::lock_guard lock(m_mutex);
					m_history.push_back(*buffer);
					m_incomingData.push(*buffer);
				}
				SendData(*buffer, socket);
				HandleClient(socket);
			}
			else
			{
				std::lock_guard lock(m_mutex);
				std::erase(m_clients, socket);
			}
		});
}

void Server::SendData(const DrawData& data, const std::shared_ptr<tcp::socket>& skipSocket)
{
	std::lock_guard lock(m_mutex);
	if (skipSocket == nullptr)
    {
        m_history.push_back(data);
    }
	for (auto& client : m_clients)
	{
		if (client != skipSocket)
		{
			boost::asio::async_write(*client, boost::asio::buffer(&data, sizeof(DrawData)),
				[](const boost::system::error_code&, std::size_t) {});
		}
	}
}

bool Server::PopData(DrawData& data)
{
	std::lock_guard lock(m_mutex);
	if (m_incomingData.empty())
	{
		return false;
	}

	data = m_incomingData.front();
	m_incomingData.pop();
	return true;
}