#pragma once
#include "assets/types/texture_2d.hpp"
#include <string>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct AppContext;

class Texture2DLoader
{
public:
	static Texture2D *Load(const std::string &filePath, AppContext* appContext);
};