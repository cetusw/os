#ifndef SIMUDUCK_IDANCEBEHAVIOR_H
#define SIMUDUCK_IDANCEBEHAVIOR_H

struct IDanceBehavior
{
    virtual ~IDanceBehavior(){}
    virtual void Dance() const = 0;
};

#endif //SIMUDUCK_IDANCEBEHAVIOR_H