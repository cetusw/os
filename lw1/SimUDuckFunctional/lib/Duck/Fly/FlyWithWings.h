#ifndef FLYWITHWINGS_H
#define FLYWITHWINGS_H

#include "IFlyBehavior.h"
#include <iostream>

class FlyWithWings : public IFlyBehavior
{
public:
	bool Fly() override
	{
		std::cout << "I'm flying with wings!!" << std::endl;
		return true;
	}
};

#endif
