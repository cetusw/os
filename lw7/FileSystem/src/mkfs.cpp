#include "FileSystemManager.h"
#include <iostream>
#include <string>

uint64_t ParseSize(const std::string& sizeStr)
{
	const uint64_t size = std::stoull(sizeStr);
	if (sizeStr.back() == 'M' || sizeStr.back() == 'm')
	{
		return size * 1024 * 1024;
	}
	if (sizeStr.back() == 'G' || sizeStr.back() == 'g')
	{
		return size * 1024 * 1024 * 1024;
	}
	return size;
}

int main(const int argc, char* argv[])
{
	if (argc < 6)
	{
		std::cout << "Usage: mkfs <image_path> --size <size> --max-files <count>" << std::endl;
		return 1;
	}

	const std::string path = argv[1];
	const uint64_t size = ParseSize(argv[3]);
	const uint32_t maxFiles = std::stoul(argv[5]);

	FileSystemManager fs;
	if (!fs.CreateImage(path, size, maxFiles))
	{
		return 1;
	}
	std::cout << "Image created successfully: " << path << std::endl;
	return 0;
}