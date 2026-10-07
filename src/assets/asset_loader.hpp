#pragma once

#include <memory>
#include <string>

class Renderer;

struct ID3D11Device;
struct ID3D11DeviceContext;

class AssetManager;
class Texture2D;
class Model;
class AudioClip;
class Font;

class VertexShader;
class PixelShader;
class ComputeShader;
class GeometryShader;
class HullShader;
class DomainShader;

class AssetLoader
{
	ID3D11Device *device;
	ID3D11DeviceContext *deviceContext;
	AssetManager *assetManager;

public:
	AssetLoader() = default;
	~AssetLoader() = default;

	bool Initialize(
		ID3D11Device *device, 
		ID3D11DeviceContext *deviceContext, 
		AssetManager *assetManager
	);

	template<class T>
	std::shared_ptr<T> Load(const std::string &path);

    std::shared_ptr<Texture2D> CreateTexture1x1(const uint8_t colour[4], bool isRGB, const std::string &name);
};

template<>
std::shared_ptr<Texture2D> AssetLoader::Load<Texture2D>(const std::string &path);

template<>
std::shared_ptr<Model> AssetLoader::Load<Model>(const std::string &path);

template<>
std::shared_ptr<AudioClip> AssetLoader::Load<AudioClip>(const std::string& path);

template<>
std::shared_ptr<Font> AssetLoader::Load<Font>(const std::string& path);

#pragma region Shaders

template<>
std::shared_ptr<VertexShader> AssetLoader::Load<VertexShader>(const std::string &path);

template<>
std::shared_ptr<PixelShader> AssetLoader::Load<PixelShader>(const std::string &path);

template<>
std::shared_ptr<ComputeShader> AssetLoader::Load<ComputeShader>(const std::string &path);

template<>
std::shared_ptr<GeometryShader> AssetLoader::Load<GeometryShader>(const std::string &path);

template<>
std::shared_ptr<DomainShader> AssetLoader::Load<DomainShader>(const std::string &path);

#pragma endregion