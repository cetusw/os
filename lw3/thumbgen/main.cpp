#include <algorithm>
#include <atomic>
#include <boost/asio/post.hpp>
#include <boost/asio/thread_pool.hpp>
#include <chrono>
#include <cmath>
#include <cxxopts.hpp>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_RESIZE2_IMPLEMENTATION
#include "stb_image_resize2.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

constexpr int CHANNELS = 3;
constexpr float GAMMA = 2.2f;
constexpr char SIZE_DELIMITER = 'x';
const std::vector<std::string> EXTENSIONS = { ".jpg", ".jpeg", ".png" };
constexpr std::string RESULT_EXTENSION = ".png";
constexpr std::string THUMB_SUFFIX = "_thumb";
const std::string INVALID_SIZE_ERROR = "Неверный формат размера. Ожидается WxH";

struct Size
{
	int width = 0;
	int height = 0;
};

struct Image
{
	std::vector<float> pixelData;
	Size size;
	int channels = 0;
};

struct Config
{
	std::filesystem::path inputDir;
	std::filesystem::path outputDir;
	Size size;
	int numThreads{};
};

Size ParseSize(const std::string& stringSize)
{
	if (stringSize.empty())
	{
		throw std::invalid_argument(INVALID_SIZE_ERROR);
	}
	const size_t delimiterPosition = stringSize.find(SIZE_DELIMITER);
	if (delimiterPosition == std::string::npos)
	{
		throw std::invalid_argument(INVALID_SIZE_ERROR);
	}
	const std::string stringWidth = stringSize.substr(0, delimiterPosition);
	const std::string stringHeight = stringSize.substr(delimiterPosition + 1);
	Size size{};
	size.width = std::stoi(stringWidth);
	size.height = std::stoi(stringHeight);
	if (size.width <= 0 || size.height <= 0)
	{
		throw std::invalid_argument("Ширина и высота должны быть положительными.");
	}
	return size;
}

Config ParseArguments(const int argc, char** argv)
{
	cxxopts::Options options("thumbgen", "Пакетная генерация миниатюр");
	options.add_options()
	("j", "Количество потоков", cxxopts::value<int>()->default_value("1"))
	("size", "Размер миниатюр (WxH)", cxxopts::value<std::string>())
	("h", "Показать справку");
	options.add_options("positional")
	("inputDir", "Каталог с исходными изображениями", cxxopts::value<std::string>())
	("outputDir", "Каталог для сохранения миниатюр", cxxopts::value<std::string>());
	options.parse_positional({ "inputDir", "outputDir" });

	const auto result = options.parse(argc, argv);

	if (result.count("h")
		|| !result.count("inputDir")
		|| !result.count("outputDir")
		|| !result.count("size"))
	{
		std::cout << options.help() << std::endl;
		exit(0);
	}

	Config config;
	config.inputDir = result["inputDir"].as<std::string>();
	config.outputDir = result["outputDir"].as<std::string>();
	config.size = ParseSize(result["size"].as<std::string>());
	config.numThreads = result["j"].as<int>();
	if (config.numThreads < 1)
	{
		throw std::invalid_argument("Число потоков должно быть не меньше 1.");
	}
	return config;
}

float Normalize(const unsigned char value)
{
	return static_cast<float>(value) / 255.0f;
}

unsigned char Denormalize(const float value)
{
	const float clampedValue = std::clamp(value, 0.0f, 1.0f);
	return static_cast<unsigned char>(clampedValue * 255.0f);
}

std::vector<unsigned char> LoadImage(const std::filesystem::path& filePath, int& width, int& height)
{
	int channels;
	unsigned char* rawData = stbi_load(
		filePath.string().c_str(),
		&width,
		&height,
		&channels,
		CHANNELS);
	if (!rawData)
	{
		std::cerr << "не удалось загрузить изображение " << filePath << std::endl;
		return {};
	}
	const size_t dataSize = width * height * CHANNELS;
	std::vector imageData(rawData, rawData + dataSize);
	stbi_image_free(rawData);
	return imageData;
}

bool SaveImage(const std::vector<unsigned char>& imageData, const Size& size, const std::filesystem::path& filePath)
{
	if (imageData.empty() || size.width <= 0 || size.height <= 0)
	{
		return false;
	}
	const int success = stbi_write_png(
		filePath.string().c_str(),
		size.width,
		size.height,
		CHANNELS,
		imageData.data(),
		size.width * CHANNELS);
	if (!success)
	{
		std::cerr << "не удалось сохранить изображение " << filePath << std::endl;
		return false;
	}
	return true;
}

std::vector<float> ConvertSRGBToLinearRGB(const std::vector<unsigned char>& srgbData)
{
	std::vector<float> linearData(srgbData.size());
	for (size_t i = 0; i < srgbData.size(); ++i)
	{
		linearData[i] = std::pow(Normalize(srgbData[i]), GAMMA);
	}
	return linearData;
}

std::vector<unsigned char> ConvertLinearRGBToSRGB(const std::vector<float>& linearData)
{
	std::vector<unsigned char> srgbData(linearData.size());
	for (size_t i = 0; i < linearData.size(); ++i)
	{
		const float srgb = std::pow(linearData[i], 1.0f / GAMMA);
		srgbData[i] = Denormalize(srgb);
	}
	return srgbData;
}

Image ResizeImage(const std::vector<float>& linearData, const Size& currentSize, const Size& newSize)
{
	Image resizedImage;
	resizedImage.size = newSize;
	resizedImage.channels = CHANNELS;
	resizedImage.pixelData.resize(static_cast<size_t>(newSize.width) * newSize.height * CHANNELS);

	stbir_resize_float_linear(
		linearData.data(),
		currentSize.width,
		currentSize.height,
		0,
		resizedImage.pixelData.data(),
		newSize.width,
		newSize.height,
		0,
		STBIR_RGB);

	return resizedImage;
}

void ProcessImage(
	const std::filesystem::path& inputPath,
	const std::filesystem::path& outputPath,
	const Size& newSize,
	std::atomic<int>& successCount,
	std::atomic<int>& errorCount)
{
	Size currentSize;
	const auto srgbData = LoadImage(inputPath, currentSize.width, currentSize.height);
	if (srgbData.empty())
	{
		++errorCount;
		return;
	}
	const auto linearData = ConvertSRGBToLinearRGB(srgbData);
	const auto resizedImage = ResizeImage(linearData, currentSize, newSize);
	const auto finalSrgbData = ConvertLinearRGBToSRGB(resizedImage.pixelData);

	std::filesystem::create_directories(outputPath.parent_path());
	if (SaveImage(finalSrgbData, resizedImage.size, outputPath))
	{
		++successCount;
	}
	else
	{
		++errorCount;
	}
}

std::vector<std::filesystem::path> GetImageFiles(const std::filesystem::path& inputDir)
{
	std::vector<std::filesystem::path> files;

	for (const auto& entry : std::filesystem::recursive_directory_iterator(inputDir))
	{
		if (!entry.is_regular_file())
		{
			continue;
		}
		std::string extension = entry.path().extension().string();
		std::ranges::transform(extension, extension.begin(), tolower);
		auto it = std::ranges::find(EXTENSIONS, extension);
		if (it != EXTENSIONS.end())
		{
			files.push_back(entry.path());
		}
	}

	std::ranges::sort(files);
	return files;
}

std::filesystem::path CreateOutputPath(
	const std::filesystem::path& inputPath,
	const std::filesystem::path& inputDir,
	const std::filesystem::path& outputDir)
{
	const auto relativePath = std::filesystem::relative(inputPath, inputDir);
	const auto newStem = inputPath.stem().string() + THUMB_SUFFIX;
	const auto newFileName = std::filesystem::path(newStem).replace_extension(RESULT_EXTENSION);
	return outputDir / relativePath.parent_path() / newFileName;
}

void GenerateResizedImages(const Config& config, const std::vector<std::filesystem::path>& files)
{
	std::atomic processedCount = 0;
	std::atomic failedCount = 0;
	boost::asio::thread_pool threadPool(config.numThreads);

	for (const auto& inputFile : files)
	{
		const auto outputFile = CreateOutputPath(inputFile, config.inputDir, config.outputDir);
		boost::asio::post(threadPool, [&, inputFile, outputFile] {
			ProcessImage(inputFile, outputFile, config.size, processedCount, failedCount);
		});
	}
	threadPool.join();

	std::cout << "processed=" << processedCount << " failed=" << failedCount << std::endl;
}

int main(const int argc, char* argv[])
{
	try
	{
		const Config config = ParseArguments(argc, argv);
		const auto imageFiles = GetImageFiles(config.inputDir);
		if (imageFiles.empty())
		{
			std::cout << "Не найдено изображений для обработки." << std::endl;
			std::cout << "processed=0 failed=0" << std::endl;
			return 0;
		}

		const auto start = std::chrono::high_resolution_clock::now();

		GenerateResizedImages(config, imageFiles);

		const auto end = std::chrono::high_resolution_clock::now();
		const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

		std::cout << "Время выполнения для "
				  << config.numThreads << " потоков: "
				  << duration.count() << " мс"
				  << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
	return 0;
}