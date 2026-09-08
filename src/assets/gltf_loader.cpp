#include "assets/gltf_loader.hpp"
#include "core/logging.hpp"
#include "assets/types/model.hpp"
#include "assets/asset_manager.hpp"
#include "assets/types/texture_2d.hpp"

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

	for (size_t meshIndex = 0; meshIndex < this->model.meshes_count; ++meshIndex)
	{
		const tg3_mesh &tMesh = this->model.meshes[meshIndex];

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
			subMeshCount++;

			for (uint32_t i = 0; i < indicies.size(); ++i)
			{
				meshIndicies.push_back(i + indexOffset);
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
	}

	ReadMaterials(this->materials);
	ReadTextures(device, deviceContext);

	tg3_model_free(&this->model);
	tg3_error_stack_free(&errors);

	LogInfo("Created %zu Meshes, %d Sub Meshes with %d Vertices, %d Indicies", this->meshes.size(), subMeshCount, this->vertexCount, this->indexCount);
	LogInfo("Created %zu Materials", this->materials.size());

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

void GLTFLoader::ReadTextures(ID3D11Device *device, ID3D11DeviceContext *deviceContext)
{
	for (uint32_t i = 0; i < this->model.images_count; ++i)
	{
		const tg3_image &tImage = this->model.images[i];
		LogInfo("Image: %s as %s", tImage.name.data, tImage.mime_type.data);
		
		Texture2D *texture = new Texture2D(
			(char*)tImage.image.data,
			tImage.width,
			tImage.height,
			tImage.component,
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