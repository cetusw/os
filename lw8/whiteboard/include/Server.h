#ifndef CALC_SERVER_H
#define CALC_SERVER_H

#include "DrawData.h"
#include <boost/asio.hpp>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

using boost::asio::ip::tcp;

class Server
{
public:
	explicit Server(short port);
	~Server();

	void Start();
	void SendData(const DrawData& data, const std::shared_ptr<tcp::socket>& skipSocket = nullptr);
	bool PopData(DrawData& data);

private:
	void Accept();
	void HandleClient(const std::shared_ptr<tcp::socket>& socket);

	boost::asio::io_context m_ioContext;
	tcp::acceptor m_acceptor;
	std::vector<std::shared_ptr<tcp::socket>> m_clients;
	std::vector<DrawData> m_history;
	std::mutex m_mutex;
	std::jthread m_networkThread;
	std::queue<DrawData> m_incomingData;
};

#endif // CALC_SERVER_H