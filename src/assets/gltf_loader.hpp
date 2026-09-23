#pragma once
#include "tinygltf/tiny_gltf_v3.h"
#include "assets/types/model.hpp"
#include "assets/types/texture_2d.hpp"

#include <string>
#include <vector>
#include <DirectXMath.h>
#include <array>

using namespace DirectX;

struct ID3D11Device;
struct ID3D11DeviceContext;

class AssetManager;
class Texture2D;

class GLTFLoader
{
	ID3D11Device *device = nullptr;
	ID3D11DeviceContext *deviceContext = nullptr;
    AssetManager *assetManager = nullptr;

	tg3_parse_options opts = {};
	tg3_error_stack errors = {};
	tg3_model model = {};

	uint32_t vertexCount = 0;
	uint32_t indexCount = 0;
    uint32_t subMeshCount = 0;

	std::vector<Mesh *> meshes = {};
	std::vector<Material> materials = {};
	std::vector<std::shared_ptr<Texture2D>> textures = {};

    std::array<int32_t, +DefaultTexture::COUNT> defaultTextureIndices = {};
    int32_t defaultMaterialIndex = -1;

    int32_t GetImageIndex(int32_t textureIndex) const;
    int32_t GetDefaultTextureIndex(DefaultTexture type);
    int32_t GetDefaultMaterialIndex();
    void AssignDefaultTextures(Material &material);

	void ReadMaterials();
	void ReadTextures();

	void ReadPrimitiveAtrributes(
		const tg3_primitive &primitive,
		std::vector<DirectX::XMFLOAT3> &positions,
		std::vector<DirectX::XMFLOAT3> &normals,
		std::vector<DirectX::XMFLOAT2> &uvs
	);
	void ReadPrimitiveIndicies(const tg3_primitive &primitive, std::vector<uint32_t> &indicies);

	void ReadScenes();
	void ReadNode(int32_t nodeIndex, XMMATRIX parentMatrix);
	void ReadMesh(int32_t meshIndex, XMMATRIX matrix);

public:
	GLTFLoader() = default;
	~GLTFLoader() = default;

	bool Load(const std::string &path, ID3D11Device* device, ID3D11DeviceContext *deviceContext, AssetManager* assetManager);

	std::vector<Mesh*> &GetMeshes() { return this->meshes; }
	std::vector<Material> &GetMaterials() { return this->materials; }
	std::vector<std::shared_ptr<Texture2D>> &GetTextures() { return this->textures; }
};