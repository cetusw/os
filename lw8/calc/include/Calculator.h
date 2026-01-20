#ifndef CALC_CALCULATOR_H
#define CALC_CALCULATOR_H

#include <string>
#include <vector>

class Calculator
{
public:
	static std::string ProcessCommand(const std::string& command);

private:
	static std::vector<int> ParseNumbers(const std::string& input);
	static std::string Calculate(char operation, const std::vector<int>& nums);
};

#endif // CALC_CALCULATOR_H
