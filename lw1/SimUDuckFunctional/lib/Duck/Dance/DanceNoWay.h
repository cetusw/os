#ifndef SIMUDUCK_DANCENOWAY_H
#define SIMUDUCK_DANCENOWAY_H

#include "IDanceBehavior.h"

class DanceNoWay : public IDanceBehavior
{
public:
    void Dance() const override {}
};

#endif //SIMUDUCK_DANCENOWAY_H