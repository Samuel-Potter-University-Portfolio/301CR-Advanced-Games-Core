#pragma once
#include "Core\Types.h"
#include <SFML\Graphics.hpp>


/**
* Replaces parts of a texture with a given colour
* @param image			Image to change
* @param fromColour		The colour to change
* @param toColour		Multiply by
*/
inline void CastColour(sf::Image& image, const sf::Color& fromColour, const sf::Color& toColour)
{
	for (uint32 x = 0; x < image.getSize().x; ++x)
		for (uint32 y = 0; y < image.getSize().y; ++y)
		{
			sf::Color pc = image.getPixel(x, y);
			if (pc == sf::Color(170, 170, 170) || pc == sf::Color(127, 127, 127)) // Multiply these colours by the given colour
				image.setPixel(x, y, pc * toColour);
		}
}

/**
* Cast all common greys to this colour
* @param image		The image to replace the colours with
* @param
*/
inline void CastColourFromCommonGrey(sf::Image& image, const sf::Color& colour)
{
	CastColour(image, sf::Color(170, 170, 170), colour);
	CastColour(image, sf::Color(127, 127, 127), colour);
}