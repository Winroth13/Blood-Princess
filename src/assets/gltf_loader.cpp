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

static constexpr std::string TG3ToString(const tg3_str &str)
{
    return str.data ? std::string(str.data, str.len) : std::string();
}

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
    this->assetManager = assetManager;

	this->opts = {};
	this->errors = {};
	this->model = {};

	this->meshes = {};
	this->materials = {};
	this->textures = {};

    this->defaultTextureIndices.fill(-1);
    this->defaultMaterialIndex = -1;

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
			errorMessage << (stderr, "[%d] %s\n", static_cast<int>(errors.entries[i].severity),
				errors.entries[i].message ? errors.entries[i].message : "(null)");
		}
		LogUnindent();
		LogError("Failed to parse %s: %s", path.c_str(), errorMessage.str().c_str());
		tg3_error_stack_free(&errors);

		return false;
	}

	this->ReadMaterials();
	this->ReadTextures();

    for (Material &material : this->materials)
        this->AssignDefaultTextures(material);

	this->ReadScenes();

	tg3_model_free(&this->model);
	tg3_error_stack_free(&errors);

	LogInfo("Created %zu Meshes, %d Sub Meshes with %d Vertices, %d Indicies", this->meshes.size(), subMeshCount, this->vertexCount, this->indexCount);
	LogInfo("Created %zu Materials", this->materials.size());
	LogInfo("Created %zu Textures", this->textures.size());

	LogUnindent();
	return true;
}

int32_t GLTFLoader::GetImageIndex(int32_t textureIndex) const 
{
    if (textureIndex < 0)
        return -1;

    if (static_cast<uint32_t>(textureIndex) >= this->model.textures_count)
    {
        LogWarn("Texture index '%d' was out of range", textureIndex);
        return -1;
    }

    const int32_t imageIndex = this->model.textures[textureIndex].source;
    if (imageIndex < 0 || static_cast<uint32_t>(imageIndex) >= this->model.images_count) 
    {
        LogWarn("Texture '%d' has no image", textureIndex);
        return -1;
    }

    return imageIndex;
}

int32_t GLTFLoader::GetDefaultTextureIndex(DefaultTexture type) 
{
    int32_t &index = this->defaultTextureIndices[+type];
    if (index < 0) 
    {
        index = static_cast<int32_t>(this->textures.size());
        this->textures.push_back(this->assetManager->GetDefaultTexture(type));
    }

    return index;
}

int32_t GLTFLoader::GetDefaultMaterialIndex() 
{
    if (this->defaultMaterialIndex < 0) 
    {
        Material material{};
        material.name = "Default";
        AssignDefaultTextures(material);

        this->defaultMaterialIndex = static_cast<int32_t>(this->materials.size());
        this->materials.push_back(material);
    }

    return this->defaultMaterialIndex;
}

void GLTFLoader::AssignDefaultTextures(Material &material) 
{
    if (material.albedoTextureIndex < 0)
        material.albedoTextureIndex = GetDefaultTextureIndex(DefaultTexture::ALBEDO);

    if (material.normalTextureIndex < 0)
        material.normalTextureIndex = GetDefaultTextureIndex(DefaultTexture::NORMAL);

    if (material.metallicRoughnessTextureIndex < 0)
        material.metallicRoughnessTextureIndex = GetDefaultTextureIndex(DefaultTexture::METALLIC_ROUGHNESS);
}

void GLTFLoader::ReadMaterials()
{
	for (uint32_t matIndex = 0; matIndex < this->model.materials_count; ++matIndex)
	{
		const tg3_material &tMaterial = this->model.materials[matIndex];

		Material material = {};
		material.name = TG3ToString(tMaterial.name);

		const tg3_pbr_metallic_roughness &pbr = tMaterial.pbr_metallic_roughness;
		
		material.baseColor = {
			static_cast<float>(pbr.base_color_factor[0]),
			static_cast<float>(pbr.base_color_factor[1]),
			static_cast<float>(pbr.base_color_factor[2]),
			static_cast<float>(pbr.base_color_factor[3])
		};

        material.albedoTextureIndex = GetImageIndex(pbr.base_color_texture.index);
        material.normalTextureIndex = GetImageIndex(tMaterial.normal_texture.index);
        material.metallicRoughnessTextureIndex = GetImageIndex(pbr.metallic_roughness_texture.index);

		this->materials.push_back(material);
	}
}

void GLTFLoader::ReadTextures()
{
	for (uint32_t i = 0; i < this->model.images_count; ++i)
	{
		const tg3_image &tImage = this->model.images[i];
        const std::string imageName = TG3ToString(tImage.name);
		
		bool isColor = false;
        DefaultTexture textureType = DefaultTexture::METALLIC_ROUGHNESS;
		for (const Material &material : this->materials)
		{
			if (static_cast<int32_t>(i) == material.albedoTextureIndex)
			{
				isColor = true;
                textureType = DefaultTexture::ALBEDO;
				break;
			}

            if (static_cast<int32_t>(i) == material.normalTextureIndex)
                textureType = DefaultTexture::NORMAL;
		}

		const unsigned char *compressedData = nullptr;
		int dataSize = 0;

		if (tImage.buffer_view >= 0 && static_cast<uint32_t>(tImage.buffer_view) < this->model.buffer_views_count)
		{
			const tg3_buffer_view &bufferView = this->model.buffer_views[tImage.buffer_view];
			const tg3_buffer &buffer = this->model.buffers[bufferView.buffer];

            if (bufferView.byte_offset + bufferView.byte_length <= buffer.data.count)
            {
			    compressedData = buffer.data.data + bufferView.byte_offset;
			    dataSize = static_cast<int>(bufferView.byte_length);
            }
		}
		else if (tImage.uri.data != nullptr && tImage.uri.len > 0)
		{
			LogWarn("External URI textures are not implemented yet: %s (using default)", TG3ToString(tImage.uri).c_str());
		}

		int width = 0, height = 0, channels = 0;
        unsigned char *pixelData = nullptr;
        
        if (compressedData && dataSize > 0)
        {
            pixelData = stbi_load_from_memory(
			    compressedData,
			    dataSize,
			    &width,
			    &height,
			    &channels,
			    STBI_rgb_alpha
		    );
        }

        std::shared_ptr<Texture2D> texture = nullptr;

		if (pixelData)
		{
            texture = std::make_shared<Texture2D>();
            bool result = texture->Create(
                (char *)pixelData,
                width,
                height,
                STBI_rgb_alpha,
                imageName,
                isColor,
                device,
                deviceContext
            );
            if (!result)
                texture = nullptr;
		}

        if (!texture)
        {
            LogWarn("Failed to load image %u '%s' (using default)", i, imageName.c_str());
            texture = this->assetManager->GetDefaultTexture(textureType);
        }

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
    if (nodeIndex < 0 || static_cast<uint32_t>(nodeIndex) >= this->model.nodes_count) 
    {
        LogWarn("Node index '%d' was out of range", nodeIndex);
        return;
    }

	const tg3_node &node = this->model.nodes[nodeIndex];

	LogInfo("Node '%s'", TG3ToString(node.name).c_str());
	LogIndent();

    XMMATRIX localMatrix;

    if (node.has_matrix)
    {
        XMFLOAT4X4 matrix;
        for (int i = 0; i < 16; ++i)
            matrix.m[i / 4][i % 4] = static_cast<float>(node.matrix[i]);

        localMatrix = XMLoadFloat4x4(&matrix);
    }
    else 
    {
        localMatrix = XMMatrixScaling(
            static_cast<float>(node.scale[0]), 
            static_cast<float>(node.scale[1]),
            static_cast<float>(node.scale[2])
        );

        XMVECTOR quaternion = XMVectorSet(
            static_cast<float>(node.rotation[0]), 
            static_cast<float>(node.rotation[1]), 
            static_cast<float>(node.rotation[2]), 
            static_cast<float>(node.rotation[3])
        );
        localMatrix *= XMMatrixRotationQuaternion(quaternion);

        localMatrix *= XMMatrixTranslation(
            static_cast<float>(node.translation[0]),
            static_cast<float>(node.translation[1]),
            static_cast<float>(node.translation[2])
    	);
    }

	const XMMATRIX matrix = XMMatrixMultiply(localMatrix, parentMatrix);

	const int32_t meshIndex = node.mesh;
	if (meshIndex != -1)
	{
		ReadMesh(meshIndex, node.skin != -1 ? XMMatrixIdentity() : matrix);
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
		LogWarn("Light node is unsupported");
	}

	for (size_t childIndex = 0; childIndex < node.children_count; ++childIndex)
	{
		const int32_t childNodeIndex = node.children[childIndex];
		ReadNode(childNodeIndex, matrix);
	}

	LogUnindent();
}

static void ComputeTangents(std::vector<Vertex> &vertices, const std::vector<UINT> &indices) {
	std::vector<XMFLOAT3> tangents(vertices.size(), { 0.0f, 0.0f, 0.0f });
	std::vector<XMFLOAT3> bitangents(vertices.size(), { 0.0f, 0.0f, 0.0f });

	for (size_t i = 0; i + 2 < indices.size(); i += 3) {
		UINT i0 = indices[i];
		UINT i1 = indices[i + 1];
		UINT i2 = indices[i + 2];

		const Vertex &v0 = vertices[i0];
		const Vertex &v1 = vertices[i1];
		const Vertex &v2 = vertices[i2];

		XMVECTOR p0 = XMLoadFloat3(&v0.position);
		XMVECTOR p1 = XMLoadFloat3(&v1.position);
		XMVECTOR p2 = XMLoadFloat3(&v2.position);

		XMVECTOR e1 = p1 - p0;
		XMVECTOR e2 = p2 - p0;

		float du1 = v1.uv.x - v0.uv.x;
		float dv1 = v1.uv.y - v0.uv.y;
		float du2 = v2.uv.x - v0.uv.x;
		float dv2 = v2.uv.y - v0.uv.y;

		// https://en.wikipedia.org/wiki/Cramer%27s_rule
		float det = du1 * dv2 - du2 * dv1;
		if (fabsf(det) < 1e-6f)
			continue;

		XMVECTOR t = (e1 * dv2 - e2 * dv1) / det;
		XMVECTOR b = (e2 * du1 - e1 * du2) / det;

		XMFLOAT3 tf;
		XMStoreFloat3(&tf, t);
		XMFLOAT3 bf;
		XMStoreFloat3(&bf, b);

		tangents[i0].x += tf.x;
		tangents[i0].y += tf.y;
		tangents[i0].z += tf.z;

		bitangents[i0].x += bf.x;
		bitangents[i0].y += bf.y;
		bitangents[i0].z += bf.z;

		tangents[i1].x += tf.x;
		tangents[i1].y += tf.y;
		tangents[i1].z += tf.z;

		bitangents[i1].x += bf.x;
		bitangents[i1].y += bf.y;
		bitangents[i1].z += bf.z;

		tangents[i2].x += tf.x;
		tangents[i2].y += tf.y;
		tangents[i2].z += tf.z;

		bitangents[i2].x += bf.x;
		bitangents[i2].y += bf.y;
		bitangents[i2].z += bf.z;
	}

	for (size_t i = 0; i < vertices.size(); ++i) {
		Vertex &v = vertices[i];

		XMVECTOR n = XMLoadFloat3(&v.normal);
		XMVECTOR t = XMLoadFloat3(&tangents[i]);
		XMVECTOR b = XMLoadFloat3(&bitangents[i]);

		// https://en.wikipedia.org/wiki/Gram%E2%80%93Schmidt_process
		XMVECTOR to = t - n * XMVector3Dot(n, t);

		if (XMVectorGetX(XMVector3LengthSq(to)) < 1e-6f) {
			XMVECTOR absN = XMVectorAbs(n);
			float absNX = XMVectorGetX(absN);
			float absNY = XMVectorGetY(absN);
			float absNZ = XMVectorGetZ(absN);

			if (absNX <= absNY && absNX <= absNZ)
				b = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
			else if (absNY <= absNZ)
				b = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
			else
				b = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

			to = XMVector3Normalize(XMVector3Cross(n, b));
			b = XMVector3Cross(n, to);
		}
		else {
			to = XMVector3Normalize(to);
		}

		XMVECTOR cross = XMVector3Cross(n, to);
		float handedness = XMVectorGetX(XMVector3Dot(cross, b)) < 0.0f ? -1.0f : 1.0f;

		XMFLOAT3 tf;
		XMStoreFloat3(&tf, to);

		v.tangent.x = tf.x;
		v.tangent.y = tf.y;
		v.tangent.z = tf.z;
		v.tangent.w = handedness;
	}
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
			positions[i].x *= -1; // GLTF uses right-handed coordinate system.

			XMVECTOR normal = XMLoadFloat3(&normals[i]);
			normal = XMVector3TransformNormal(normal, inverseTransposeMatrix);
			normal = XMVector3Normalize(normal);
			XMStoreFloat3(&normals[i], normal);
			normals[i].x *= -1; // GLTF uses right-handed coordinate system.
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

		for (int32_t i = static_cast<int32_t>(indicies.size()) - 1; i >= 0; --i)
		{
			meshIndicies.push_back(indicies[i] + indexOffset);
		}
		indexCount += static_cast<uint32_t>(indicies.size());
	}

    ComputeTangents(meshVertices, meshIndicies);

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
