#include "assets/gltf_loader.hpp"
#include "core/logging.hpp"
#include "assets/types/model.hpp"
#include "assets/asset_manager.hpp"
#include "assets/types/texture_2d.hpp"
#include "stb_image/stb_image.h"

#include <d3d11.h>
#include <sstream>
#include <vector>
#include <unordered_map>


template<typename T>
std::vector<T> ReadAccessorData(tg3_model *model, int32_t accessorIndex)
{
	if (accessorIndex == -1)
	{
		return {};
	}

	const tg3_accessor &accessor = model->accessors[accessorIndex];
	const tg3_buffer_view &bufferView = model->buffer_views[accessor.buffer_view];
	const tg3_buffer &buffer = model->buffers[bufferView.buffer];

	const uint8_t *baseData = buffer.data.data;
	uint64_t offset = bufferView.byte_offset + accessor.byte_offset;
	const T *typedData = reinterpret_cast<const T *>(baseData + offset);

	int32_t stride = tg3_accessor_byte_stride(&accessor, &bufferView);
	int32_t elementStride = stride > 0 ? stride / sizeof(T) : sizeof(T);

	std::vector<T> result;

	uint32_t componentsPerElement = 1;
	if (accessor.type == TG3_TYPE_VEC2) componentsPerElement = 2;
	else if (accessor.type == TG3_TYPE_VEC3) componentsPerElement = 3;
	else if (accessor.type == TG3_TYPE_VEC4) componentsPerElement = 4;
	else if (accessor.type == TG3_TYPE_MAT2) componentsPerElement = 4;
	else if (accessor.type == TG3_TYPE_MAT3) componentsPerElement = 9;
	else if (accessor.type == TG3_TYPE_MAT4) componentsPerElement = 16;

	uint64_t totalComponents = accessor.count * componentsPerElement;
	result.reserve(totalComponents);

	for (uint64_t i = 0; i < totalComponents; i++)
	{
		result.push_back(typedData[i]);
	}

	return result;
}

bool GLTFLoader::Load(
	const std::string &path, 
	ID3D11Device *device,
	ID3D11DeviceContext* deviceContext,
	AssetManager *assetManager
)
{
	LogInfo("Parsing file...");
	LogIndent();

	this->device = device;
	this->deviceContext = deviceContext;

	this->opts = {};
	this->errors = {};
	this->model = {};

	this->meshes = {};
	this->materials = {};
	this->textures = {};

	this->vertexCount = 0;
	this->indexCount = 0;
	uint32_t subMeshCount = 0;

	tg3_parse_options_init(&opts);
	tg3_error_stack_init(&errors);

	tg3_error_code error = tg3_parse_file(
		&this->model,
		&this->errors,
		path.c_str(),
		static_cast<uint32_t>(path.size()),
		&this->opts
	);

	if (error != TG3_OK)
	{
		std::stringstream errorMessage;
		for (uint32_t i = 0; i < errors.count; i++)
		{
			errorMessage << (stderr, "[%d] %s\n", (int)errors.entries[i].severity,
				errors.entries[i].message ? errors.entries[i].message : "(null)");
		}
		LogUnindent();
		LogError("Failed to parse %s: %s", path.c_str(), errorMessage.str().c_str());
		tg3_error_stack_free(&errors);

		return false;
	}

	ReadScenes();
	ReadMaterials(this->materials);
	ReadTextures();

	tg3_model_free(&this->model);
	tg3_error_stack_free(&errors);

	LogInfo("Created %zu Meshes, %d Sub Meshes with %d Vertices, %d Indicies", this->meshes.size(), subMeshCount, this->vertexCount, this->indexCount);
	LogInfo("Created %zu Materials", this->materials.size());
	LogInfo("Created %zu Textures", this->textures.size());

	LogUnindent();
	return true;
}

void GLTFLoader::ReadMaterials(std::vector<Material> &materials)
{
	for (uint32_t matIndex = 0; matIndex < this->model.materials_count; ++matIndex)
	{
		const tg3_material &tMaterial = this->model.materials[matIndex];

		Material material = {};
		material.name = std::string(tMaterial.name.data, tMaterial.name.len);

		const tg3_pbr_metallic_roughness &pbr = tMaterial.pbr_metallic_roughness;
		
		material.baseColor = {
			static_cast<float>(pbr.base_color_factor[0]),
			static_cast<float>(pbr.base_color_factor[1]),
			static_cast<float>(pbr.base_color_factor[2]),
			static_cast<float>(pbr.base_color_factor[3])
		};

		material.colorTextureIndex = pbr.base_color_texture.index;
		material.normalTextureIndex = tMaterial.normal_texture.index;

		materials.push_back(material);
	}
}

void GLTFLoader::ReadTextures()
{
	for (uint32_t i = 0; i < this->model.images_count; ++i)
	{
		const tg3_image &tImage = this->model.images[i];
		
		const unsigned char *compressedData = nullptr;
		int dataSize = 0;

		if (tImage.buffer_view >= 0 && static_cast<uint32_t>(tImage.buffer_view) < this->model.buffer_views_count)
		{
			const tg3_buffer_view &bufferView = this->model.buffer_views[tImage.buffer_view];
			const tg3_buffer &buffer = this->model.buffers[bufferView.buffer];

			compressedData = buffer.data.data + bufferView.byte_offset;
			dataSize = static_cast<int>(bufferView.byte_length);
		}
		else if (tImage.uri.data != nullptr && tImage.uri.len > 0)
		{
			LogError("External URI textures are not implemented yet: %s", tImage.uri.data);
			continue;
		}

		if (!compressedData || dataSize <= 0)
		{
			LogError("Failed to find data for image %s", tImage.name.data);
			continue;
		}

		int width, height, channels = 0;
		unsigned char *pixelData = stbi_load_from_memory(
			compressedData,
			dataSize,
			&width,
			&height,
			&channels,
			STBI_rgb_alpha
		);

		if (!pixelData)
		{
			LogError("STBI failed to decode image %s", tImage.name.data);
			continue;
		}

		Texture2D *texture = new Texture2D();

		// TODO: Should check if this fails.
		bool result = texture->Create(
			(char *)pixelData,
			width,
			height,
			channels,
			"?",
			device,
			deviceContext
		);

		this->textures.push_back(texture);
	}
}

void GLTFLoader::ReadPrimitiveAtrributes(
	const tg3_primitive &primitive, 
	std::vector<DirectX::XMFLOAT3> &positions, 
	std::vector<DirectX::XMFLOAT3> &normals, 
	std::vector<DirectX::XMFLOAT2> &uvs
)
{
	for (uint32_t attrIndex = 0; attrIndex < primitive.attributes_count; ++attrIndex)
	{
		const tg3_str_int_pair &attr = primitive.attributes[attrIndex];
		std::string_view attrName(attr.key.data, attr.key.len);

		if (attrName == "POSITION")
		{
			std::vector<float> positionsData = ReadAccessorData<float>(&this->model, attr.value);
			for (size_t i = 0; i < positionsData.size(); i += 3)
			{
				positions.push_back({
					positionsData[i],
					positionsData[i + 1],
					positionsData[i + 2]
					});
			}
		}
		else if (attrName == "NORMAL")
		{
			std::vector<float> normalsData = ReadAccessorData<float>(&this->model, attr.value);
			for (size_t i = 0; i < normalsData.size(); i += 3)
			{
				normals.push_back({
					normalsData[i],
					normalsData[i + 1],
					normalsData[i + 2]
					});
			}
		}
		else if (attrName == "TEXCOORD_0")
		{
			std::vector<float> uvsData = ReadAccessorData<float>(&this->model, attr.value);
			for (size_t i = 0; i < uvsData.size(); i += 2)
			{
				uvs.push_back({
					uvsData[i],
					uvsData[i + 1]
					});
			}
		}
		else
		{
			LogWarn("Unsupported attribute: %s", attrName.data());
		}
	}
}

void GLTFLoader::ReadPrimitiveIndicies(const tg3_primitive &primitive, std::vector<uint32_t> &indicies)
{
	if (primitive.indices == -1)
	{
		LogWarn("Primitive has no indicies");
		return;
	}

	const tg3_accessor &indexAccessor = this->model.accessors[primitive.indices];

	switch (indexAccessor.component_type)
	{
		case TG3_COMPONENT_TYPE_UNSIGNED_SHORT:
		{
			std::vector<uint16_t> shortIndices = ReadAccessorData<uint16_t>(&this->model, primitive.indices);
			indicies.assign(shortIndices.begin(), shortIndices.end());
		}
		break;

		case TG3_COMPONENT_TYPE_UNSIGNED_INT:
			indicies = ReadAccessorData<uint32_t>(&this->model, primitive.indices);
			break;

		default:
			LogError("Unsupported index component type: %d", indexAccessor.component_type);
			break;
	}
}

void GLTFLoader::ReadScenes()
{
	for (size_t sceneIndex = 0; sceneIndex < this->model.scenes_count; ++sceneIndex)
	{
		const tg3_scene &tScene = this->model.scenes[sceneIndex];
		LogInfo("Scene '%s'", std::string(tScene.name.data, tScene.name.len).c_str());
		LogIndent();

		for (size_t i = 0; i < tScene.nodes_count; ++i)
		{
			const int32_t nodeIndex = tScene.nodes[i];
			XMMATRIX matrix = XMMatrixIdentity();

			ReadNode(nodeIndex, matrix);
		}

		LogUnindent();
	}
}

void GLTFLoader::ReadNode(int32_t nodeIndex, XMMATRIX parentMatrix)
{
	const tg3_node &node = this->model.nodes[nodeIndex];

	LogInfo("Node '%s'", std::string(node.name.data, node.name.len).c_str());
	LogIndent();

	XMMATRIX matrix = XMMatrixScaling(
		static_cast<float>(node.scale[0]), 
		static_cast<float>(node.scale[1]),
		static_cast<float>(node.scale[2])
	);
	matrix *= XMMatrixRotationRollPitchYaw(
		static_cast<float>(node.rotation[0]), 
		static_cast<float>(node.rotation[1]),
		static_cast<float>(node.rotation[2])
	);
	matrix *= XMMatrixTranslation(
		static_cast<float>(node.translation[0]),
		static_cast<float>(node.translation[1]),
		static_cast<float>(node.translation[2])
	);
	matrix = XMMatrixMultiply(parentMatrix, matrix);

	const int32_t meshIndex = node.mesh;
	if (meshIndex != -1)
	{
		ReadMesh(meshIndex, matrix);
	}

	if (node.camera != -1)
	{
		LogWarn("Camera node is unsupported");
	}

	if (node.emitter != -1)
	{
		LogWarn("Emitter node is unsupported");
	}

	if (node.light != -1)
	{
		LogWarn("Emitter node is unsupported");
	}

	for (size_t childIndex = 0; childIndex < node.children_count; ++childIndex)
	{
		const int32_t childNodeIndex = node.children[childIndex];
		ReadNode(childNodeIndex, matrix);
	}

	LogUnindent();
}

void GLTFLoader::ReadMesh(int32_t meshIndex, XMMATRIX matrix)
{
	const tg3_mesh &tMesh = this->model.meshes[meshIndex];
	LogInfo("Mesh '%s'", std::string(tMesh.name.data, tMesh.name.len).c_str());
	LogIndent();

	XMMATRIX inverseTransposeMatrix = matrix;
	inverseTransposeMatrix.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	inverseTransposeMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, inverseTransposeMatrix));

	std::vector<Vertex> meshVertices = {};
	std::vector<uint32_t> meshIndicies = {};
	std::vector<SubMesh> subMeshes = {};

	for (size_t primIndex = 0; primIndex < tMesh.primitives_count; ++primIndex)
	{
		const tg3_primitive &primitive = tMesh.primitives[primIndex];
		
		std::vector<DirectX::XMFLOAT3> positions = {};
		std::vector<DirectX::XMFLOAT3> normals = {};
		std::vector<DirectX::XMFLOAT2> uvs = {};
		std::vector<uint32_t> indicies = {};

		ReadPrimitiveAtrributes(primitive, positions, normals, uvs);
		ReadPrimitiveIndicies(primitive, indicies);

		for (size_t i = 0; i < positions.size(); ++i)
		{
			XMVECTOR position = XMVector3Transform(XMLoadFloat3(&positions[i]), matrix);
			XMStoreFloat3(&positions[i], position);

			XMVECTOR normal = XMLoadFloat3(&normals[i]);
			normal = XMVector3TransformNormal(normal, inverseTransposeMatrix);
			normal = XMVector3Normalize(normal);
			XMStoreFloat3(&normals[i], normal);
		}
		
		for (size_t i = 0; i < positions.size(); ++i)
		{
			Vertex vertex(
				positions[i],
				normals[i],
				uvs[i]
			);
			meshVertices.push_back(vertex);
		}
		vertexCount += static_cast<uint32_t>(meshVertices.size());

		uint32_t indexOffset = static_cast<uint32_t>(meshIndicies.size());

		SubMesh subMesh = {};
		subMesh.startIndex = indexOffset;
		subMesh.numIndicies = static_cast<uint32_t>(indicies.size());
		subMesh.materialIndex = primitive.material;
		subMeshes.push_back(subMesh);

		for (uint32_t i = 0; i < indicies.size(); ++i)
		{
			meshIndicies.push_back(indicies[i] + indexOffset);
		}
		indexCount += static_cast<uint32_t>(indicies.size());
	}

	Mesh *mesh = new Mesh();
	mesh->Create(
		std::string(tMesh.name.data, tMesh.name.len),
		meshVertices,
		meshIndicies,
		subMeshes,
		device
	);
	meshes.push_back(mesh);

	LogUnindent();
}
