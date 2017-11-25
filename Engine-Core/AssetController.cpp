#include "Includes\Core\AssetController.h"
#include "Includes\Core\Logger.h"


AssetController::~AssetController()
{
	for (auto& it : m_fonts)
		delete it.second;

	for (auto& it : m_animations)
		delete it.second;

	for (auto& it : m_textures)
		delete it.second;
	LOG("Assets destroyed");
}

inline static string GetKey(const string& path)
{
	// Store key as lowercase
	string key = path;
	std::transform(key.begin(), key.end(), key.begin(), ::tolower); 
	return key;
}

void AssetController::HandleUpdate(const float& deltaTime)
{
	for (auto& it : m_animations)
		it.second->UpdateAnimation(deltaTime);
}



void AssetController::RegisterTexture(const string& path, sf::Texture* texture) 
{
#ifdef BUILD_CLIENT
	const string key = GetKey(path);

	if (m_textures.find(key) != m_textures.end())
	{
		LOG_WARNING("Multiple entries for texture '%s'", path.c_str());
		delete texture;
	}
	else
	{
		m_textures[key] = texture;
		//LOG("\t-Registered texture at '%s'", key.c_str());
	}
#else
	delete texture;
#endif
}

void AssetController::RegisterTexture(const string& path, bool isSmoothed, bool isRepeated) 
{
#ifdef BUILD_CLIENT

	sf::Texture* texture = new sf::Texture;
	if (!texture->loadFromFile(path))
	{
		LOG_ERROR("Failed to load texture at '%s'", path.c_str());
		return;
	}

	texture->setSmooth(isSmoothed);
	texture->setRepeated(isRepeated);
	RegisterTexture(path, texture);
#endif
}

const sf::Texture* AssetController::GetTexture(const string& path) const
{
#ifdef BUILD_CLIENT
	const string key = GetKey(path);

	auto it = m_textures.find(key);
	if (it == m_textures.end())
		return nullptr;
	else
		return it->second;
#else
	return nullptr;
#endif
}



void AssetController::RegisterAnimation(const string& path, AnimationSheet* animation) 
{
#ifdef BUILD_CLIENT
	const string key = GetKey(path);

	if (m_animations.find(key) != m_animations.end())
	{
		LOG_WARNING("Multiple entries for animation '%s'", path.c_str());
		delete animation;
	}
	else
	{
		m_animations[key] = animation;
		//LOG("\t-Registered animation at '%s'", key.c_str());
	}
#else
	delete animation;
#endif
}

const AnimationSheet* AssetController::GetAnimation(const string& path) const 
{
#ifdef BUILD_CLIENT
	const string key = GetKey(path);

	auto it = m_animations.find(key);
	if (it == m_animations.end())
		return nullptr;
	else
		return it->second;
#else
	return nullptr;
#endif
}


void AssetController::RegisterFont(const string& path, sf::Font* font)
{
#ifdef BUILD_CLIENT
	const string key = GetKey(path);

	if (m_fonts.find(key) != m_fonts.end())
	{
		LOG_WARNING("Multiple entries for font '%s'", path.c_str());
		delete font;
	}
	else
	{
		m_fonts[key] = font;
		//LOG("\t-Registered font at '%s'", key.c_str());
	}
#else
	delete font;
#endif
}

void AssetController::RegisterFont(const string& path)
{
#ifdef BUILD_CLIENT

	sf::Font* font = new sf::Font;
	if (!font->loadFromFile(path))
	{
		LOG_ERROR("Failed to load font at '%s'", path.c_str());
		return;
	}

	RegisterFont(path, font);
#endif
}

const sf::Font* AssetController::GetFont(const string& path) const
{
#ifdef BUILD_CLIENT
	const string key = GetKey(path);

	auto it = m_fonts.find(key);
	if (it == m_fonts.end())
		return nullptr;
	else
		return it->second;
#else
	return nullptr;
#endif
}