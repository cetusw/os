#ifndef SIMUDUCK_DANCEMINUET_H
#define SIMUDUCK_DANCEMINUET_H
#include <iostream>
#include <ostream>

#include "IDanceBehavior.h"

class DanceMinuet : public IDanceBehavior
{
public:
    void Dance() const override
    {
        std::cout << "I'm dancing a minuet now!" << std::endl;
    }
};

#endif //SIMUDUCK_DANCEMINUET_H