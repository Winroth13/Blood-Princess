#pragma once
#include "tinygltf/tiny_gltf_v3.h"
#include "assets/types/model.hpp"

#include <string>
#include <vector>
#include <DirectXMath.h>

struct ID3D11Device;

class GLTFLoader
{
	tg3_parse_options opts = {};
	tg3_error_stack errors = {};
	tg3_model model = {};

	uint32_t vertexCount = 0;
	uint32_t indexCount = 0;

	std::vector<Mesh*> meshes = {};
	std::vector<Material> materials = {};

	void ReadMaterials(std::vector<Material> &materials);

	void ReadPrimitiveAtrributes(
		const tg3_primitive &primitive,
		std::vector<DirectX::XMFLOAT3> &positions,
		std::vector<DirectX::XMFLOAT3> &normals,
		std::vector<DirectX::XMFLOAT2> &uvs
	);
	void ReadPrimitiveIndicies(const tg3_primitive &primitive, std::vector<uint32_t> &indicies);

public:
	GLTFLoader() = default;
	~GLTFLoader() = default;

	bool Load(const std::string &path, ID3D11Device* device);

	std::vector<Mesh*> &GetMeshes() { return this->meshes; }
	std::vector<Material> &GetMaterials() { return this->materials; }
};