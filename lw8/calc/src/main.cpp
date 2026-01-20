#include "Server.h"
#include "Client.h"
#include <iostream>

int main(const int argc, char* argv[])
{
	try
	{
		if (argc == 2)
		{
			const int port = std::stoi(argv[1]);
			Server server(10);
			server.Start(port);
		}
		else if (argc == 3)
		{
			const std::string addr = argv[1];
			const int port = std::stoi(argv[2]);
			Client client(addr, port);
			client.Run();
		}
		else
		{
			std::cout << "Usage:\n Server: calc PORT\n Client: calc ADDRESS PORT" << std::endl;
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Fatal error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}