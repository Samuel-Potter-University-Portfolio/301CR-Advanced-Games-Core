#pragma once
#include "Common.h"
#include "EngineMemory.h"

#include <unordered_map>
#include <SFML\Graphics.hpp>


#define TEX_SMOOTH	1
#define TEX_REPEAT	1

/**
* Holds all assets, so that duplicates don't have to be made
*/
class CORE_API AssetController
{
private:
	std::unordered_map<string, sf::Texture*> m_textures;
public:
	~AssetController();

	/**
	* Loads and registers this texture
	* @param path			URL to this texture
	* @param isSmoothed		Should the texture enable smooth filter or not
	* @param isRepeated		Should the texture repeat/tile
	*/
	void RegisterTexture(const string& path, bool isSmoothed = true, bool isRepeated = false);
	/**
	* Registers this texture (Forfeits memory rights over this texture to asset controller)
	* @param path		URL to this texture
	*/
	void RegisterTexture(const string& path, sf::Texture* texture);

	/**
	* Retreives a texture at this given path
	* @param path		URL to the image file
	* @returns The texture or nullptr if not registered
	*/
	const sf::Texture* GetTexture(const string& path) const;
};

