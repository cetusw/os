#include "FileSystemManager.h"
#include "MountManager.h"

int main(const int argc, char* argv[])
{
	FileSystemManager fsManager;
	const MountManager mountManager(fsManager);

	return mountManager.Run(argc, argv);
}
