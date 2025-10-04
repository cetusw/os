#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <cerrno>
#include <fstream>
#include <algorithm>

constexpr std::string OUT_EXTENSION = ".out";
constexpr size_t BUFFER_SIZE = 4024;
constexpr size_t CHILDREN_LIMIT = 10;

void PrintUsage()
{
    std::cerr << "Usage: flip-case <file1> [file2] ...\n";
}

std::vector<std::string> ParseArguments(const int argc, char *argv[])
{
    std::vector<std::string> inputFiles;
    for (int i = 1; i < argc; i++)
    {
        inputFiles.emplace_back(argv[i]);
    }

    return inputFiles;
}

char FlipCase(const char c)
{
    if (c >= 'a' && c <= 'z')
    {
        return c - 'a' + 'A';
    }
    if (c >= 'A' && c <= 'Z')
    {
        return c - 'A' + 'a';
    }
    return c;
}

bool ReadAndWrite(std::ifstream &inputFile, std::ofstream &outputFile, const std::string &fileName)
{
    char buffer[BUFFER_SIZE];
    while (inputFile.read(buffer, BUFFER_SIZE) || inputFile.gcount() > 0)
    {
        const size_t count = inputFile.gcount();

        for (size_t i = 0; i < count; i++)
        {
            buffer[i] = FlipCase(buffer[i]);
        }

        outputFile.write(buffer, count);
        if (!outputFile)
        {
            std::cerr << "Error writing to output file " << fileName << ": " << strerror(errno) << std::endl;
            return false;
        }
    }
    return true;
}

int ProcessFile(const std::string &inputPath)
{
    const pid_t pid = getpid();
    std::cout << "Process " << pid << " is processing " << inputPath << std::endl;

    std::ifstream inputFile(inputPath, std::ios::binary);
    if (!inputFile)
    {
        std::cerr << "Error opening input file " << inputPath << ": " << strerror(errno) << std::endl;
        return 1;
    }

    std::string outputPath = inputPath + OUT_EXTENSION;
    std::ofstream outputFile(outputPath, std::ios::binary);
    if (!outputFile)
    {
        std::cerr << "Error creating output file " << outputPath << ": " << strerror(errno) << std::endl;
        return 1;
    }

    if (!ReadAndWrite(inputFile, outputFile, outputPath))
    {
        return 1;
    }

    if (inputFile.bad())
    {
        std::cerr << "Error reading from input file " << inputPath << ": " << strerror(errno) << std::endl;
        return 1;
    }

    std::cout << "Process " << pid << " has finished writing to " << outputPath << std::endl;
    return 0;
}

bool WaitProcess(std::vector<pid_t> &childrenPids)
{
    int status;
    pid_t finishedPid;
    do
    {
        finishedPid = waitpid(-1, &status, 0);
    } while (finishedPid == -1 && errno == EINTR);

    if (finishedPid == -1)
    {
        std::cerr << "Error in waitpid: " << strerror(errno) << std::endl;
        return true;
    }

    std::cout << "Child process " << finishedPid << " is over" << std::endl;

    const auto it = std::ranges::find(childrenPids, finishedPid);
    if (it != childrenPids.end())
    {
        childrenPids.erase(it);
    }

    if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
    {
        return false;
    }
    if (WIFSIGNALED(status))
    {
        return false;
    }
    return true;
}

bool LaunchChildProcess(const std::vector<std::string> &inputFiles, const size_t fileIndex,
                        std::vector<pid_t> &childrenPids)
{
    const pid_t pid = fork();
    if (pid == -1)
    {
        std::cerr << "Error forking: " << strerror(errno) << std::endl;
        return false;
    }
    if (pid == 0)
    {
        const int result = ProcessFile(inputFiles[fileIndex]);
        _Exit(result);
    }
    childrenPids.push_back(pid);

    return true;
}

bool ManageChildren(const std::vector<std::string> &inputFiles, std::vector<pid_t> &childrenPids)
{
    for (size_t currentFile = 0; currentFile < inputFiles.size(); ++currentFile)
    {
        while (childrenPids.size() >= CHILDREN_LIMIT)
        {
            if (!WaitProcess(childrenPids))
            {
                return false;
            }
        }

        if (!LaunchChildProcess(inputFiles, currentFile, childrenPids))
        {
            return false;
        }
    }
    return true;
}

bool WaitForAllChildren(std::vector<pid_t> &childrenPids)
{
    while (!childrenPids.empty())
    {
        if (!WaitProcess(childrenPids))
        {
            return false;
        }
    }
    return true;
}

int main(const int argc, char *argv[])
{
    if (argc < 2)
    {
        PrintUsage();
        return 2;
    }

    const std::vector<std::string> inputFiles = ParseArguments(argc, argv);
    std::vector<pid_t> childrenPids;

    if (!ManageChildren(inputFiles, childrenPids))
    {
        return 1;
    }

    if (!WaitForAllChildren(childrenPids))
    {
        return 1;
    }

    return 0;
}
