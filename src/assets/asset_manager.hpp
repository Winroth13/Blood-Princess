#pragma once
#include <unordered_map>
#include <string>
#include <memory>

#include "assets/types/texture_2d.hpp"

struct AppContext;

class AssetManager
{
private:
	std::unordered_map<std::string, std::unique_ptr<Texture2D>> texture2dAssets;
	AppContext *appContext;

public:
	bool Initialize(AppContext* appContext);
	void Shutdown();

	Texture2D* GetTexture2D(const std::string &filePath);
};