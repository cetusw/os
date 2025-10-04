#ifndef EXE_PACKER_EXEPACKER_H
#define EXE_PACKER_EXEPACKER_H

#include <cstdint>
#include <vector>
#include <string>

constexpr const char* SIGNATURE = "SFX!";
constexpr const char* SELF_EXE = "/proc/self/exe";
constexpr __mode_t EXECUTABLE_PERMISSIONS = 0755;
constexpr int BUFFER_SIZE = 4096;

struct PayloadHeader
{
    uint32_t originalSize;
    uint32_t compressedSize;
    char signature[4];
};

class ExePacker
{
public:
    explicit ExePacker(std::string selfPath);

    [[nodiscard]] bool HasPayload() const;

    [[nodiscard]] int RunAsPacker(const std::string &inputPath, const std::string &outputPath);

    int RunAsUnpacker(int argc, char *argv[]) const;

private:
    std::string selfPath;

    bool ReadPayload(std::vector<char> &payloadData, PayloadHeader &header) const;

    [[nodiscard]] static std::vector<char> ReadFile(const std::string &path);

    static bool CompressData(const std::vector<char> &input, std::vector<char> &output);

    static bool DecompressData(const std::vector<char> &input, uint32_t compressedSize, uint32_t originalSize,
                               std::vector<char> &output);

    [[nodiscard]] static std::string CreateTempFile(const std::vector<char> &data);

    static int ExecuteTempFile(const std::string &tempPath, int argc, char *argv[]);
};


#endif //EXE_PACKER_EXEPACKER_H
