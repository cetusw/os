#ifndef WHITEBOARD_CLIENT_H
#define WHITEBOARD_CLIENT_H

#include "DrawData.h"
#include <boost/asio.hpp>
#include <string>
#include <queue>
#include <mutex>
#include <thread>

using boost::asio::ip::tcp;

class Client
{
public:
	Client(const std::string& host, short port);
	~Client();

	void Start();
	void SendData(const DrawData& data);
	bool PopData(DrawData& data);

private:
	void Read();

	boost::asio::io_context m_ioContext;
	tcp::socket m_socket;

	std::queue<DrawData> m_incomingData;
	std::mutex m_mutex;
	std::jthread m_networkThread;
};

#endif // WHITEBOARD_CLIENT_H
