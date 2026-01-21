#ifndef WHITEBOARD_COLOR_H
#define WHITEBOARD_COLOR_H

#include <SFML/Graphics.hpp>

class Color
{
public:
	static sf::Color FromUint(const uint32_t color)
	{
		return {
			static_cast<sf::Uint8>(color >> 24 & 0xFF),
			static_cast<sf::Uint8>(color >> 16 & 0xFF),
			static_cast<sf::Uint8>(color >> 8 & 0xFF),
			static_cast<sf::Uint8>(color & 0xFF)
		};
	}

	static uint32_t ToUint(const sf::Color color)
	{
		return (static_cast<uint32_t>(color.r) << 24)
			| (static_cast<uint32_t>(color.g) << 16)
			| (static_cast<uint32_t>(color.b) << 8)
			| static_cast<uint32_t>(color.a);
	}
};

#endif // WHITEBOARD_COLOR_H
