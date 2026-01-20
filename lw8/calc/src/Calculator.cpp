#include "Calculator.h"
#include <sstream>

constexpr char ADD_OPERATION = '+';
constexpr char SUB_OPERATION = '-';

std::string Calculator::ProcessCommand(const std::string& command)
{
	if (command.empty())
	{
		return "ERROR: Empty command";
	}

	const char operation = command[0];
	const std::string numbersPart = command.substr(1);

	try
	{
		const std::vector<int> nums = ParseNumbers(numbersPart);
		if (nums.empty())
		{
			return "ERROR: No numbers provided";
		}
		return Calculate(operation, nums);
	}
	catch (...)
	{
		return "ERROR: Invalid number format";
	}
}

std::vector<int> Calculator::ParseNumbers(const std::string& input)
{
	std::vector<int> nums;
	std::stringstream ss(input);
	int n;
	while (ss >> n)
	{
		nums.push_back(n);
	}
	if (!ss.eof() && ss.fail())
	{
		throw std::runtime_error("Invalid number format or character detected");
	}
	return nums;
}

std::string Calculator::Calculate(const char operation, const std::vector<int>& nums)
{
	int result = nums[0];
	if (operation == ADD_OPERATION)
	{
		for (size_t i = 1; i < nums.size(); ++i)
		{
			result += nums[i];
		}
	}
	else if (operation == SUB_OPERATION)
	{
		for (size_t i = 1; i < nums.size(); ++i)
		{
			result -= nums[i];
		}
	}
	else
	{
		return "ERROR: Unknown operator";
	}

	return std::to_string(result);
}
