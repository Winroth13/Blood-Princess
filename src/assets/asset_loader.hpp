#pragma once

#include <memory>
#include <string>

class Renderer;

class Texture2D;
class Model;

class AssetLoader
{
    // TODO: Just store device and deviceContext directly instead?
    const Renderer *renderer;

public:
	AssetLoader() = default;
	~AssetLoader() = default;

	bool Initialize(const Renderer *renderer);

	template<class T>
	std::shared_ptr<T> Load(const std::string &path);
};

template<>
std::shared_ptr<Texture2D> AssetLoader::Load<Texture2D>(const std::string &path);

template<>
std::shared_ptr<Model> AssetLoader::Load<Model>(const std::string &path);