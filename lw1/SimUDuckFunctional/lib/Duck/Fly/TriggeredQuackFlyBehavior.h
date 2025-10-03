#ifndef SIMUDUCK_TRIGGEREDQUACKFLYBEHAVIOR_H
#define SIMUDUCK_TRIGGEREDQUACKFLYBEHAVIOR_H
#include <iostream>
#include <memory>

#include "IFlyBehavior.h"
#include "../Quack/IQuackBehavior.h"

class TriggeredQuackFlyBehavior : public IFlyBehavior
{
public:
    TriggeredQuackFlyBehavior(
        std::unique_ptr<IFlyBehavior> wrappedFlyBehavior,
        std::unique_ptr<IQuackBehavior> quackBehavior)
        : m_wrappedFlyBehavior(std::move(wrappedFlyBehavior))
          , m_quackBehavior(std::move(quackBehavior))
          , m_flightCount(0)
    {
    }

    bool Fly() override
    {
        if (!m_wrappedFlyBehavior->Fly())
        {
            return false;
        }
        m_flightCount++;

        if (IsEvenFlight(m_flightCount))
        {
            m_quackBehavior->Quack();
        }

        std::cout << "Порядковый номер вылета: " << m_flightCount << std::endl;

        return true;
    }

private:
    static bool IsEvenFlight(const int flightNumber)
    {
        return flightNumber > 0 && flightNumber % 2 == 0;
    }

    std::unique_ptr<IFlyBehavior> m_wrappedFlyBehavior;
    std::unique_ptr<IQuackBehavior> m_quackBehavior;
    int m_flightCount;
};

#endif //SIMUDUCK_TRIGGEREDQUACKFLYBEHAVIOR_H
