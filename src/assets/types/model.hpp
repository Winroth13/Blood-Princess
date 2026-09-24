#pragma once
#include "assets/asset.hpp"

#include <vector>
#include <memory>
#include <DirectXMath.h>
#include <string>
#include <d3d11.h>

class Texture2D;
class AnimationLibrary;
class Skeleton;

#pragma region Vertex
struct Vertex
{
	DirectX::XMFLOAT3 position = { 0, 0, 0 };
	DirectX::XMFLOAT3 normal = { 0, 0, 0 };
	DirectX::XMFLOAT2 uv = { 0, 0 };
	DirectX::XMFLOAT4 tangent = { 0, 0, 0, 0 };

	Vertex(
		const DirectX::XMFLOAT3 position,
		const DirectX::XMFLOAT3 normal,
		const DirectX::XMFLOAT2 uv
	) : position(position), normal(normal), uv(uv)
	{
	}
};

bool operator== (const Vertex &v1, const Vertex &v2);

/*
* We need to define a hash function for our Vertex struct so we can
* detect duplicates when loading models.
*/
namespace std {
	template <>
	struct hash<Vertex>
	{
		size_t operator()(const Vertex &v) const
		{
			size_t seed = 0;
			std::hash<float> hasher;
			auto combine = [&](float f)
				{
					seed ^= hasher(f) + (seed << 6) + (seed >> 2) + 0x9e3779b9;
				};

			combine(v.position.x);
			combine(v.position.y);
			combine(v.position.z);

			combine(v.normal.x);
			combine(v.normal.y);
			combine(v.normal.z);

			combine(v.uv.x);
			combine(v.uv.y);

			return seed;
		}
	};
}

#pragma endregion

#pragma region Submesh
struct SubMesh
{
	uint32_t startIndex;
	uint32_t numIndicies;
	int32_t materialIndex;
};
#pragma endregion

#pragma region Mesh
class Mesh
{
	std::string name = "?";
	std::vector<SubMesh> subMeshes = {};

	size_t numVertices = 0;
	size_t numIndicies = 0;

	ID3D11Buffer *vertexBuffer = nullptr;
	ID3D11Buffer *indexBuffer = nullptr;

public:
	Mesh() = default;
	~Mesh();

	bool Create(
		const std::string &name,
		const std::vector<Vertex> &vertices,
		const std::vector<uint32_t> &indicies,
		const std::vector<SubMesh> &subMeshes,
		ID3D11Device *device
	);

	size_t GetNumVertices() const { return this->numVertices; }
	size_t GetNumIndicies() const { return this->numIndicies; }

	ID3D11Buffer *GetVertexBuffer() { return this->vertexBuffer; }
	ID3D11Buffer *GetIndexBuffer() { return this->indexBuffer; }

    const std::vector<SubMesh> &GetSubMeshes() const { return this->subMeshes; }

};
#pragma endregion

#pragma region Material
struct Material
{
	std::string name;
	DirectX::XMFLOAT4 baseColor = { 1, 1, 1, 1 };

    // Should always be valid after loading bc default textures
	int32_t albedoTextureIndex = -1;
	int32_t normalTextureIndex = -1;
	int32_t metallicRoughnessTextureIndex = -1;
};
#pragma endregion

#pragma region Model

class Model : public Asset
{
	std::vector<std::unique_ptr<Mesh>> meshes;
	std::vector<Material> materials;
	std::vector<std::shared_ptr<Texture2D>> textures;

    std::shared_ptr<AnimationLibrary> animations;
    std::shared_ptr<Skeleton> skeleton;

public:
	Model() = default;
	~Model();

	void DrawImGui() override;

	bool Create(
		const std::vector<Mesh*> &meshes, 
		const std::vector<Material> &materials,
		const std::vector<std::shared_ptr<Texture2D>> &textures,
        std::shared_ptr<AnimationLibrary> animations,
        std::shared_ptr<Skeleton> skeleton
	);

    const std::vector<std::unique_ptr<Mesh>> &GetMeshes() const { return this->meshes; }
    const std::vector<Material> &GetMaterials() const { return this->materials; }
    const std::vector<std::shared_ptr<Texture2D>> &GetTextures() const { return this->textures; }

    bool IsSkinned() const { return this->skeleton != nullptr; }
    std::shared_ptr<AnimationLibrary> GetAnimation() const { return this->animations; }
    std::shared_ptr<Skeleton> GetSkeleton() const { return this->skeleton; }

	Texture2D *GetTexture(int32_t index);
};

#pragma endregion