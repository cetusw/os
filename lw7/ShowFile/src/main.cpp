#include "FatSystem.h"
#include <iostream>

int main(const int argc, char* argv[])
{
	if (argc != 3)
	{
		std::cerr << "Usage: show-file <image_path> <inner_path>\n";
		return 1;
	}

	try
	{
		const std::string imgPath = argv[1];
		std::string targetPath = argv[2];

		const FatSystem fs(imgPath);
		fs.ShowPath(targetPath);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << "\n";
		return 1;
	}

	return 0;
}