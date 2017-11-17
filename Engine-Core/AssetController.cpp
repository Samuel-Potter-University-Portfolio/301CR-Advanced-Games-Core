#include "Includes\Core\AssetController.h"
#include "Includes\Core\Logger.h"


AssetController::~AssetController()
{
	for (auto& it : m_textures)
		delete it.second;
	LOG("Assets destroyed");
}

void AssetController::RegisterTexture(const string& path, sf::Texture* texture) 
{
#ifdef BUILD_CLIENT
	// Store key as lowercase
	string key = path;
	std::transform(key.begin(), key.end(), key.begin(), ::tolower);

	if (m_textures.find(key) != m_textures.end())
	{
		LOG_WARNING("Multiple entries for texture '%s'", path.c_str());
		delete texture;
	}
	else
	{
		m_textures[key] = texture;
		LOG("\t-Registered texture at '%s'", key.c_str());
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
	// Store key as lowercase
	string key = path;
	std::transform(key.begin(), key.end(), key.begin(), ::tolower);

	auto it = m_textures.find(key);
	if (it == m_textures.end())
		return nullptr;
	else
		return it->second;
#else
	return nullptr;
#endif
}