#include "Whiteboard.h"
#include <iostream>
#include <string>

int main(const int argc, char* argv[])
{
	try
	{
		Whiteboard whiteboard;
		if (argc == 2)
		{
			whiteboard.RunServer(static_cast<short>(std::stoi(argv[1])));
		}
		else if (argc == 3)
		{
			whiteboard.RunClient(argv[1], static_cast<short>(std::stoi(argv[2])));
		}
		else
		{
			std::cout << "Usage Server: whiteboard PORT\nUsage Client: whiteboard ADDR PORT" << std::endl;
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
	return 0;
}