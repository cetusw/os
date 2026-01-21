#ifndef WHITEBOARD_COLOR_H
#define WHITEBOARD_COLOR_H

#include <SFML/Graphics.hpp>

class Color
{
public:
	static sf::Color FromUint(const unsigned int color)
	{
		return {
			static_cast<sf::Uint8>(color >> 24 & 0xFF),
			static_cast<sf::Uint8>(color >> 16 & 0xFF),
			static_cast<sf::Uint8>(color >> 8 & 0xFF),
			static_cast<sf::Uint8>(color & 0xFF)
		};
	}

	static unsigned int ToUint(const sf::Color color)
	{
		return (color.r << 24)
			| (color.g << 16)
			| (color.b << 8)
			| color.a;
	}
};

#endif // WHITEBOARD_COLOR_H
