#ifndef SIMUDUCK_DANCETHEWALTZ_H
#define SIMUDUCK_DANCETHEWALTZ_H
#include <iostream>
#include <ostream>

#include "IDanceBehavior.h"

class DanceWaltz : public IDanceBehavior
{
public:
    void Dance() const override
    {
        std::cout << "I'm dancing the waltz now!" << std::endl;
    }
};

#endif //SIMUDUCK_DANCETHEWALTZ_H
