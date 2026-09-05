#pragma once
#include <memory>
#include <string>

struct AppContext;
class Texture2D;

class AssetLoader
{
	AppContext *appContext;

public:
	AssetLoader() = default;
	~AssetLoader() = default;

	bool Initialize(AppContext *appContext);

	template<class T>
	std::shared_ptr<T> Load(const std::string &path);
};

template<>
std::shared_ptr<Texture2D> AssetLoader::Load<Texture2D>(const std::string &path);