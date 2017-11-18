#pragma once
#include "Common.h"
#include "AnimationSheet.h"

#include <unordered_map>
#include <SFML\Graphics.hpp>


/**
* Holds all assets, so that duplicates don't have to be made
*/
class CORE_API AssetController
{
private:
	std::unordered_map<string, sf::Texture*> m_textures;
	std::unordered_map<string, AnimationSheet*> m_animations;
public:
	~AssetController();

	/**
	* Callback from engine for every tick by main
	* @param engine			The engine + game to update using
	* @param deltaTime		Time since last update (In seconds)
	*/
	void HandleUpdate(const float& deltaTime);


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
	* @param texture	The texture to register
	*/
	void RegisterTexture(const string& path, sf::Texture* texture);

	/**
	* Retreives a texture at this given path
	* @param path		URL to the image file
	* @returns The texture or nullptr if not registered
	*/
	const sf::Texture* GetTexture(const string& path) const;


	/**
	* Registers this animation sheet (Forfeits memory rights over this animation to asset controller)
	* @param path			URL to this animation
	* @param animation		The animation to register
	*/
	void RegisterAnimation(const string& path, AnimationSheet* animation);

	/**
	* Retreives a animation at this given path
	* @param path		URL to the animation file
	* @returns The animation or nullptr if not registered
	*/
	const AnimationSheet* GetAnimation(const string& path) const;
};

