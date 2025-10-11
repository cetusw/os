#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <thread>
#include <cmath>
#include <algorithm>
#include <mutex>
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_RESIZE2_IMPLEMENTATION
#include "stb_image_resize2.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "cxxopts.hpp"

constexpr int CHANNELS = 3;
constexpr int DEFAULT_SIZE = 256;
constexpr float GAMMA = 2.2f;
const std::vector<std::string> ALLOWED_EXTENSIONS = {".png", ".jpeg", ".jpg"};
constexpr double NORMALIZING_FACTOR = DEFAULT_SIZE * DEFAULT_SIZE;

struct Config
{
    std::string queryPath;
    std::string inputDir;
    int numThreads{};
    int top{};
    double mseThreshold{};
};

struct Image
{
    std::vector<float> pixelData;
    int width{};
    int height{};
    int channels{};
};

struct Result
{
    double mse{};
    std::filesystem::path filePath;

    bool operator<(const Result &other) const
    {
        return mse < other.mse;
    }
};

unsigned char *GetRawImageData(const std::filesystem::path &filePath, int &width, int &height, int &channels)
{
    unsigned char *rawData = stbi_load(
        filePath.c_str(),
        &width,
        &height,
        &channels,
        CHANNELS
    );
    if (!rawData)
    {
        throw std::runtime_error("Не удалось загрузить изображение: " + filePath.string());
    }

    return rawData;
}

std::vector<float> ConvertToLinearRGB(const unsigned char *rawData, const int width, const int height)
{
    const int linerSize = width * height * CHANNELS;
    std::vector<float> linearData(linerSize);
    for (int i = 0; i < linerSize; ++i)
    {
        const float srgb = static_cast<float>(rawData[i]) / 255.0f;
        linearData[i] = std::pow(srgb, GAMMA);
    }

    return linearData;
}

Image ResizeImage(const std::vector<float> &linearData, const int width, const int height)
{
    Image resizedImage;
    resizedImage.width = DEFAULT_SIZE;
    resizedImage.height = DEFAULT_SIZE;
    resizedImage.channels = CHANNELS;
    resizedImage.pixelData.resize(DEFAULT_SIZE * DEFAULT_SIZE * CHANNELS);

    stbir_resize_float_linear(
        linearData.data(),
        width,
        height,
        0,
        resizedImage.pixelData.data(),
        DEFAULT_SIZE,
        DEFAULT_SIZE,
        0,
        STBIR_RGB
    );

    return resizedImage;
}

Image PreprocessImage(const std::filesystem::path &filePath)
{
    int width, height, channels;
    unsigned char *rawData = GetRawImageData(filePath, width, height, channels);

    const std::vector<float> linearData = ConvertToLinearRGB(rawData, width, height);
    stbi_image_free(rawData);

    return ResizeImage(linearData, width, height);
}

double CalculateMSE(const Image &firstImage, const Image &secondImage)
{
    double mse = 0.0;
    for (size_t i = 0; i < firstImage.pixelData.size(); ++i)
    {
        mse += std::pow(firstImage.pixelData[i] - secondImage.pixelData[i], 2);
    }
    return mse / static_cast<double>(firstImage.pixelData.size()) * NORMALIZING_FACTOR;
}

void ProcessFiles(const Image &queryImage,
                  const std::vector<std::filesystem::path> &filePaths,
                  std::vector<Result> &globalResults,
                  std::mutex &resultsMutex)
{
    std::vector<Result> localResults;
    for (const auto &filePath: filePaths)
    {
        try
        {
            Image candidateImage = PreprocessImage(filePath);
            const double mseValue = CalculateMSE(queryImage, candidateImage);
            localResults.push_back({mseValue, filePath});
        } catch (const std::runtime_error &e)
        {
            std::cerr << "Ошибка обработки " << filePath << ": " << e.what() << std::endl;
        }
    }

    std::lock_guard lock(resultsMutex);
    globalResults.insert(globalResults.end(), localResults.begin(), localResults.end());
}

std::vector<std::filesystem::path> CollectImagePaths(const std::string &directoryPath)
{
    std::vector<std::filesystem::path> allFiles;
    for (const auto &entry: std::filesystem::recursive_directory_iterator(directoryPath))
    {
        if (!entry.is_regular_file())
        {
            continue;
        }
        std::string extension = entry.path().extension().string();
        std::ranges::transform(extension, extension.begin(), ::tolower);

        auto it = std::ranges::find(ALLOWED_EXTENSIONS, extension);
        if (it != ALLOWED_EXTENSIONS.end())
        {
            allFiles.push_back(entry.path());
        }
    }
    std::ranges::sort(allFiles);
    return allFiles;
}

std::vector<std::vector<std::filesystem::path> > DistributePathsToThreads(
    const std::vector<std::filesystem::path> &allFiles, const int numThreads)
{
    // TODO: разделить массив файлов сразу. проверить, что так быстрее
    std::vector<std::vector<std::filesystem::path> > threadFiles(numThreads);
    for (size_t i = 0; i < allFiles.size(); ++i)
    {
        threadFiles[i % numThreads].push_back(allFiles[i]);
    }
    return threadFiles;
}

void LaunchAndJoinThreads(const int numThreads,
                          const std::vector<std::vector<std::filesystem::path> > &threadFiles,
                          const Image &queryImage,
                          std::vector<Result> &allResults,
                          std::mutex &resultsMutex)
{
    // TODO: можно использовать jthread + пояснить методы и работу detach
    std::vector<std::thread> workerThreads(numThreads);
    for (int i = 0; i < numThreads; ++i)
    {
        // TODO: изучить std::cref и std::ref
        workerThreads.emplace_back(ProcessFiles, std::cref(queryImage),
                                   std::cref(threadFiles[i]),
                                   std::ref(allResults),
                                   std::ref(resultsMutex));
    }

    for (auto &thread: workerThreads)
    {
        thread.join();
    }
}

std::vector<Result> FilterResults(std::vector<Result> &allResults, const int top, const double mseThreshold)
{
    std::sort(allResults.begin(), allResults.end());

    std::vector<Result> finalResults;
    if (mseThreshold >= 0)
    {
        for (const auto &result: allResults)
        {
            if (result.mse <= mseThreshold)
            {
                finalResults.push_back(result);
            }
        }
    } else
    {
        finalResults = allResults;
    }

    if (top > 0 && finalResults.size() > static_cast<size_t>(top))
    {
        finalResults.resize(top);
    }

    return finalResults;
}

void PrintResults(const std::vector<Result> &results)
{
    std::cout << std::fixed
            << std::setprecision(1)
            << std::left;
    for (const auto &result: results)
    {
        std::cout << std::setw(10) << result.mse << "  " << result.filePath.string() << std::endl;
    }
}

Config ParseArguments(const int argc, char **argv)
{
    cxxopts::Options options("mt-img-sim", "Многопоточный поиск похожих изображений");
    options.add_options()
            ("j", "Количество потоков", cxxopts::value<int>()->default_value("1"))
            ("top", "Вывести K наиболее похожих", cxxopts::value<int>())
            ("threshold", "Вывести с MSE <= T", cxxopts::value<double>())
            ("query", "Путь к файлу-образцу", cxxopts::value<std::string>())
            ("input_dir", "Каталог с изображениями", cxxopts::value<std::string>())
            ("h", "Показать справку");
    options.parse_positional({"query", "input_dir"});
    const auto result = options.parse(argc, argv);

    if (result.count("help") || !result.count("query") || !result.count("input_dir"))
    {
        std::cout << options.help() << std::endl;
        exit(1);
    }

    Config config;
    config.queryPath = result["query"].as<std::string>();
    config.inputDir = result["input_dir"].as<std::string>();
    config.numThreads = result["j"].as<int>();
    config.top = result.count("top") ? result["top"].as<int>() : -1;
    config.mseThreshold = result.count("threshold") ? result["threshold"].as<double>() : -1.0;
    return config;
}

int main(const int argc, char **argv)
{
    const Config config = ParseArguments(argc, argv);

    const Image queryImage = PreprocessImage(config.queryPath);

    const std::vector<std::filesystem::path> allFiles = CollectImagePaths(config.inputDir);

    const auto threadFiles = DistributePathsToThreads(allFiles, config.numThreads);

    std::vector<Result> allResults;
    std::mutex resultsMutex;

    const auto start = std::chrono::high_resolution_clock::now();

    LaunchAndJoinThreads(config.numThreads, threadFiles, queryImage, allResults, resultsMutex);

    const auto end = std::chrono::high_resolution_clock::now();

    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << config.numThreads << " " << duration.count() << std::endl;


    const std::vector<Result> finalResults = FilterResults(allResults, config.top, config.mseThreshold);
    PrintResults(finalResults);
}
