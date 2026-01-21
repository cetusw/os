#include "Client.h"

Client::Client(const std::string& host, const short port)
	: m_socket(m_ioContext)
{
	tcp::resolver resolver(m_ioContext);
	const auto endpoints = resolver.resolve(host, std::to_string(port));
	boost::asio::connect(m_socket, endpoints);
}

Client::~Client()
{
	m_ioContext.stop();
}

void Client::Start()
{
	Read();

	m_networkThread = std::jthread([this]() {
		m_ioContext.run();
	});
}

void Client::Read()
{
	auto buffer = std::make_shared<DrawData>();

	boost::asio::async_read(m_socket, boost::asio::buffer(buffer.get(), sizeof(DrawData)),
		[this, buffer](const boost::system::error_code& error, std::size_t) {
			if (!error)
			{
				{
					std::lock_guard lock(m_mutex);
					m_incomingData.push(*buffer);
				}
				Read();
			}
		});
}

void Client::SendData(const DrawData& data)
{
	boost::asio::async_write(m_socket, boost::asio::buffer(&data, sizeof(DrawData)),
		[](const boost::system::error_code&, std::size_t) {});
}

bool Client::PopData(DrawData& data)
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