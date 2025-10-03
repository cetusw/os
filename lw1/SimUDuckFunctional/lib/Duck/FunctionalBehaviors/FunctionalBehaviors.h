#ifndef SIMUDUCK_FUNCTIONALBEHAVIORS_H
#define SIMUDUCK_FUNCTIONALBEHAVIORS_H

#include <functional>
#include <iostream>

namespace Behaviors
{
    const std::function quackBehavior = [] { std::cout << "Quack Quack!!!" << std::endl; };
    const std::function squeakBehavior = [] { std::cout << "Squeek!!!" << std::endl; };
    const std::function muteQuackBehavior = []
    {
    };

    const std::function danceWaltz = [] { std::cout << "I'm dancing the waltz now!" << std::endl; };
    const std::function danceMinuet = [] { std::cout << "I'm dancing a minuet now!" << std::endl; };
    const std::function danceNoWay = []
    {
    };

    const std::function flyWithWings = [] { std::cout << "I'm flying with wings!!" << std::endl; };
    const std::function flyNoWay = []
    {
    };

    inline auto makeFlyWithWings()
    {
        return [flightsCount = 0]() mutable
        {
            flightsCount++;
            std::cout << "I'm flying with wings!! Flight number: " << flightsCount << std::endl;
        };
    }
} // namespace Behaviors

#endif //SIMUDUCK_FUNCTIONALBEHAVIORS_H
