#ifndef DUCK_H
#define DUCK_H

#include "Fly/IFlyBehavior.h"
#include "Quack/IQuackBehavior.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <vector>

#include "Dance/IDanceBehavior.h"

class Duck
{
public:
    Duck(std::unique_ptr<IFlyBehavior> &&flyBehavior,
         std::unique_ptr<IQuackBehavior> &&quackBehavior,
         std::unique_ptr<IDanceBehavior> &&danceBehavior)
    {
        SetFlyBehavior(std::move(flyBehavior));
        SetQuackBehavior(std::move(quackBehavior));
        SetDanceBehavior(std::move(danceBehavior));
    }

    void Quack() const
    {
        m_quackBehavior->Quack();
    }

    void Swim()
    {
        std::cout << "I'm swimming" << std::endl;
    }

    void Fly()
    {
        m_flyBehavior->Fly();
    }

    void Dance() const
    {
        m_danceBehavior->Dance();
    }

    void SetFlyBehavior(std::unique_ptr<IFlyBehavior> &&flyBehavior)
    {
        assert(flyBehavior);
        m_flyBehavior = std::move(flyBehavior);
    }

    void SetQuackBehavior(std::unique_ptr<IQuackBehavior> &&quackBehavior)
    {
        assert(quackBehavior);
        m_quackBehavior = std::move(quackBehavior);
    }

    void SetDanceBehavior(std::unique_ptr<IDanceBehavior> &&danceBehavior)
    {
        assert(danceBehavior);
        m_danceBehavior = std::move(danceBehavior);
    }

    virtual void Display() const = 0;

    virtual ~Duck() = default;

private:
    std::unique_ptr<IFlyBehavior> m_flyBehavior;
    std::unique_ptr<IQuackBehavior> m_quackBehavior;
    std::unique_ptr<IDanceBehavior> m_danceBehavior;
};

#endif
