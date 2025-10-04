#include "ExePacker.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <utility>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <zlib.h>

ExePacker::ExePacker(std::string selfPath) : m_selfPath(std::move(selfPath))
{
}

bool ExePacker::HasPayload() const
{
    std::vector<char> payloadData;
    PayloadHeader header{};
    return ReadPayload(payloadData, header);
}

bool ExePacker::ReadPayload(std::vector<char> &payloadData, PayloadHeader &header) const
{
    std::ifstream file(m_selfPath, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        return false;
    }

    const std::streamsize fileSize = file.tellg();
    if (fileSize < sizeof(PayloadHeader))
    {
        return false;
    }

    file.seekg(fileSize - static_cast<std::streamoff>(sizeof(PayloadHeader)));
    file.read(reinterpret_cast<char *>(&header), sizeof(PayloadHeader));

    if (memcmp(header.signature, SIGNATURE, 4) != 0)
    {
        return false;
    }

    if (header.compressedSize > fileSize - sizeof(PayloadHeader))
    {
        return false;
    }

    file.seekg(fileSize - static_cast<std::streamoff>(sizeof(PayloadHeader)) - header.compressedSize);
    payloadData.resize(header.compressedSize);
    file.read(payloadData.data(), header.compressedSize);

    return true;
}

int ExePacker::RunAsPacker(const std::string &inputPath, const std::string &outputPath)
{
    std::vector<char> inputData = ReadFile(inputPath);
    if (inputData.empty())
    {
        std::cerr << "Error reading input file: " << inputPath << std::endl;
        return 1;
    }

    std::vector<char> compressedData;
    if (!CompressData(inputData, compressedData))
    {
        std::cerr << "Error compressing data" << std::endl;
        return 1;
    }

    PayloadHeader header{};
    header.originalSize = inputData.size();
    header.compressedSize = compressedData.size();
    memcpy(header.signature, SIGNATURE, 4);

    std::ifstream self(inputPath, std::ios::binary);
    std::ofstream output(outputPath, std::ios::binary);
    std::vector<char> buffer(BUFFER_SIZE);

    std::ifstream selfFile(SELF_EXE, std::ios::binary);
    while (selfFile.read(buffer.data(), static_cast<std::streamsize>(buffer.size())))
    {
        output.write(buffer.data(), selfFile.gcount());
    }
    output.write(buffer.data(), selfFile.gcount());

    output.write(compressedData.data(), static_cast<std::streamsize>(compressedData.size()));
    output.write(reinterpret_cast<const char *>(&header), sizeof(header));

    chmod(outputPath.c_str(), EXECUTABLE_PERMISSIONS);
    return 0;
}

int ExePacker::RunAsUnpacker(const int argc, char *argv[]) const
{
    std::vector<char> payloadData;
    PayloadHeader header{};

    if (!ReadPayload(payloadData, header))
    {
        std::cerr << "Error reading payload" << std::endl;
        return 1;
    }

    std::vector<char> originalData;
    if (!DecompressData(payloadData, header.compressedSize, header.originalSize, originalData))
    {
        std::cerr << "Error decompressing payload" << std::endl;
        return 1;
    }

    const std::string tempPath = CreateTempFile(originalData);
    if (tempPath.empty())
    {
        std::cerr << "Error creating temporary file" << std::endl;
        return 1;
    }

    const int result = ExecuteTempFile(tempPath, argc, argv);
    unlink(tempPath.c_str());
    return result;
}

std::vector<char> ExePacker::ReadFile(const std::string &path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        return {};
    }
    const std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size))
    {
        return {};
    }

    return buffer;
}

bool ExePacker::CompressData(const std::vector<char> &input, std::vector<char> &output)
{
    uLong compressedSize = compressBound(input.size());
    output.resize(compressedSize);

    const int result = compress2(
        reinterpret_cast<Bytef *>(output.data()),
        &compressedSize,
        reinterpret_cast<const Bytef *>(input.data()),
        input.size(),
        Z_DEFAULT_COMPRESSION
    );

    output.resize(compressedSize);
    return result == Z_OK;
}

bool ExePacker::DecompressData(const std::vector<char> &input, const uint32_t compressedSize,
                               const uint32_t originalSize,
                               std::vector<char> &output)
{
    output.resize(originalSize);

    uLong outputSize = originalSize;
    const int result = uncompress(
        reinterpret_cast<Bytef *>(output.data()),
        &outputSize,
        reinterpret_cast<const Bytef *>(input.data()),
        compressedSize
    );

    return result == Z_OK && outputSize == originalSize;
}

std::string ExePacker::CreateTempFile(const std::vector<char> &data)
{
    char tempPath[] = "/tmp/exe_packer_XXXXXX";
    const int fd = mkstemp(tempPath);
    if (fd == -1)
    {
        return "";
    }

    if (write(fd, data.data(), data.size()) != static_cast<ssize_t>(data.size()))
    {
        close(fd);
        unlink(tempPath);
        return "";
    }

    close(fd);
    chmod(tempPath, EXECUTABLE_PERMISSIONS);
    return tempPath;
}

int ExePacker::ExecuteTempFile(const std::string &tempPath, const int argc, char *argv[])
{
    const pid_t pid = fork();
    if (pid == 0)
    {
        std::vector<char *> args;
        for (int i = 1; i < argc; i++)
        {
            args.push_back(argv[i]);
        }
        args.push_back(nullptr);

        execv(tempPath.c_str(), args.data());
        _exit(1);
    }
    if (pid > 0)
    {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
        {
            return WEXITSTATUS(status);
        }
        return 1;
    }
    std::cerr << "Error forking process" << std::endl;
    return 1;
}
