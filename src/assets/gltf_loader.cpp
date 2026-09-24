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

#undef min
#undef max

static constexpr std::string TG3ToString(const tg3_str &str)
{
    return str.data ? std::string(str.data, str.len) : std::string();
}

struct AccessorView 
{
    const uint8_t *data = nullptr;
    uint64_t elementCount = 0;
    uint32_t elementStride = 0;
    uint32_t componentSize = 0;
    uint32_t componentsPerElement = 0;
    int32_t componentType = 0;
    bool isNormalized = false;
};

static bool GetAccessorView(const tg3_model &model, int32_t accessorIndex, AccessorView &outView) 
{
    if (accessorIndex < 0 || static_cast<uint32_t>(accessorIndex) >= model.accessors_count) 
    {
        LogWarn("Accessor index '%d' was out of range", accessorIndex);
        return false;
    }

    const tg3_accessor &accessor = model.accessors[accessorIndex];

    if (accessor.sparse.is_sparse)
        LogWarn("Sparse accessor");

    if (accessor.buffer_view < 0 || static_cast<uint32_t>(accessor.buffer_view) >= model.buffer_views_count) 
    {
        LogWarn("Invalid buffer view");
        return false;
    }

    const tg3_buffer_view &bufferView = model.buffer_views[accessor.buffer_view];

    if (bufferView.buffer < 0 || static_cast<uint32_t>(bufferView.buffer) >= model.buffers_count) 
    {
        LogWarn("Invalid buffer");
        return false;
    }

    const tg3_buffer &buffer = model.buffers[bufferView.buffer];

    const int32_t componentSize = tg3_component_size(accessor.component_type);
    const int32_t componentCount = tg3_num_components(accessor.type);
    const int32_t stride = tg3_accessor_byte_stride(&accessor, &bufferView);

    if (componentSize <= 0 || componentCount <= 0 || stride <= 0) 
    {
        LogWarn("Invalid (component) type");
        return false;
    }

    const uint64_t start = bufferView.byte_offset + accessor.byte_offset;
    const uint64_t elementSize = static_cast<uint64_t>(componentSize) * componentCount;

    if (accessor.count > 0) 
    {
        const uint64_t end = start + (accessor.count - 1) * stride + elementSize;
        const uint64_t viewEnd = bufferView.byte_offset + bufferView.byte_length;

        if (end > viewEnd || end > buffer.data.count || !buffer.data.data) 
        {
            LogWarn("Accessor reads out of bounds");
            return false;
        }
    }

    outView.data = buffer.data.data + start;
    outView.elementCount = accessor.count;
    outView.elementStride = static_cast<uint32_t>(stride);
    outView.componentSize = static_cast<uint32_t>(componentSize);
    outView.componentsPerElement = static_cast<uint32_t>(componentCount);
    outView.componentType = accessor.component_type;
    outView.isNormalized = accessor.normalized != 0;

    return true;
}

static float ReadComponentFloat(const uint8_t *data, int32_t componentType, bool isNormalized) 
{
    switch (componentType) 
    {
        case TG3_COMPONENT_TYPE_FLOAT: {
            float value;
            memcpy(&value, data, sizeof(value));
            return value;
        }
        case TG3_COMPONENT_TYPE_UNSIGNED_BYTE: {
            uint8_t value = *data;
            return isNormalized ? value / 255.0f : static_cast<float>(value);
        }
        case TG3_COMPONENT_TYPE_BYTE: {
            int8_t value;
            memcpy(&value, data, sizeof(value));
            return isNormalized ? std::max(value / 127.0f, -1.0f) : static_cast<float>(value);
        }
        case TG3_COMPONENT_TYPE_UNSIGNED_SHORT: {
            uint16_t value;
            memcpy(&value, data, sizeof(value));
            return isNormalized ? value / 65535.0f : static_cast<float>(value);
        }
        case TG3_COMPONENT_TYPE_SHORT: {
            int16_t value;
            memcpy(&value, data, sizeof(value));
            return isNormalized ? std::max(value / 32767.0f, -1.0f) : static_cast<float>(value);
        }
        case TG3_COMPONENT_TYPE_UNSIGNED_INT: {
            uint32_t value;
            memcpy(&value, data, sizeof(value));
            return static_cast<float>(value);
        }

        default:
            return 0.0f;
    }
}

static uint32_t ReadComponentUnsignedInt(const uint8_t *data, int32_t componentType) 
{
    switch (componentType) 
    {
        case TG3_COMPONENT_TYPE_UNSIGNED_BYTE: {
            return *data;
        }
        case TG3_COMPONENT_TYPE_UNSIGNED_SHORT: {
            uint16_t value;
            memcpy(&value, data, sizeof(value));
            return value;
        }
        case TG3_COMPONENT_TYPE_UNSIGNED_INT: {
            uint32_t value;
            memcpy(&value, data, sizeof(value));
            return value;
        }

        default:
            return 0u;
    }
}

static bool ReadAccessorFloats(const tg3_model &model, int32_t accessorIndex, uint32_t expectedComponentPerElement, std::vector<float> &outValues) 
{
    AccessorView view;
    if (!GetAccessorView(model, accessorIndex, view))
        return false;

    if (view.componentsPerElement != expectedComponentPerElement) 
    {
        LogWarn("Accessor had '%u' components per element, expected '%u'", view.componentsPerElement, expectedComponentPerElement);
        return false;
    }

    outValues.resize(view.elementCount * view.componentsPerElement);

    for (uint64_t i = 0; i < view.elementCount; ++i)
    {
        const uint8_t *element = view.data + i * view.elementStride;
        for (uint32_t component = 0; component < view.componentsPerElement; ++component) {
            outValues[i * view.componentsPerElement + component] = ReadComponentFloat(
                element + component * view.componentSize,
                view.componentType,
                view.isNormalized
            );
        }
    }

    return true;
}

static bool ReadAccessorUnsignedInts(const tg3_model &model, int32_t accessorIndex, uint32_t expectedComponentPerElement, std::vector<uint32_t> &outValues) 
{
    AccessorView view;
    if (!GetAccessorView(model, accessorIndex, view))
        return false;

    if (view.componentsPerElement != expectedComponentPerElement) 
    {
        LogWarn("Accessor had '%u' components per element, expected '%u'", view.componentsPerElement, expectedComponentPerElement);
        return false;
    }

    if (view.componentType != TG3_COMPONENT_TYPE_UNSIGNED_BYTE &&
        view.componentType != TG3_COMPONENT_TYPE_UNSIGNED_SHORT &&
        view.componentType != TG3_COMPONENT_TYPE_UNSIGNED_INT
    ) {
        LogWarn("Component type wasn't an unsigned int");
        return false;
    }

    outValues.resize(view.elementCount * view.componentsPerElement);

    for (uint64_t i = 0; i < view.elementCount; ++i)
    {
        const uint8_t *element = view.data + i * view.elementStride;
        for (uint32_t component = 0; component < view.componentsPerElement; ++component) {
            outValues[i * view.componentsPerElement + component] = ReadComponentUnsignedInt(
                element + component * view.componentSize,
                view.componentType
            );
        }
    }

    return true;
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
	this->subMeshCount = 0;

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

	LogInfo("Created %zu Meshes, %d Sub Meshes with %d Vertices, %d Indicies", this->meshes.size(), this->subMeshCount, this->vertexCount, this->indexCount);
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

bool GLTFLoader::ReadPrimitiveAttributes(
	const tg3_primitive &primitive, 
	std::vector<DirectX::XMFLOAT3> &positions, 
	std::vector<DirectX::XMFLOAT3> &normals, 
	std::vector<DirectX::XMFLOAT2> &uvs
)
{
    std::vector<float> data;

	for (uint32_t attrIndex = 0; attrIndex < primitive.attributes_count; ++attrIndex)
	{
		const tg3_str_int_pair &attr = primitive.attributes[attrIndex];
		const std::string attrName = TG3ToString(attr.key);

		if (attrName == "POSITION")
		{
            if (!ReadAccessorFloats(this->model, attr.value, 3, data))
                continue;

            positions.resize(data.size() / 3);
            memcpy(positions.data(), data.data(), positions.size() * sizeof(XMFLOAT3));
		}
		else if (attrName == "NORMAL")
		{
            if (!ReadAccessorFloats(this->model, attr.value, 3, data))
                continue;

            normals.resize(data.size() / 3);
            memcpy(normals.data(), data.data(), normals.size() * sizeof(XMFLOAT3));
		}
		else if (attrName == "TEXCOORD_0")
		{
            if (!ReadAccessorFloats(this->model, attr.value, 2, data))
                continue;

            uvs.resize(data.size() / 2);
            memcpy(uvs.data(), data.data(), uvs.size() * sizeof(XMFLOAT2));
		}
		else
		{
			LogWarn("Unsupported attribute: %s", attrName.c_str());
		}
	}

    if (positions.empty()) 
    {
        LogWarn("Primitive had no position attributes");
        return false;
    }

    return true;
}

bool GLTFLoader::ReadPrimitiveIndices(const tg3_primitive &primitive, uint32_t vertexCount, std::vector<uint32_t> &indices)
{
	if (primitive.indices == -1)
	{
        indices.resize(vertexCount);
        for (uint32_t i = 0; i < vertexCount; ++i)
            indices[i] = i;
	}
    else if (!ReadAccessorUnsignedInts(this->model, primitive.indices, 1, indices)) 
    {
        LogWarn("Failed to read primitive indices");
        return false;
    }

    if (indices.size() % 3 != 0) 
    {
        LogWarn("Index count not a multiple of 3");
        indices.resize(indices.size() - indices.size() % 3);
    }

    for (uint32_t index : indices)
    {
        if (index >= vertexCount) 
        {
            LogWarn("Index out of bounds; index '%u', vertex count '%u'", index, vertexCount);
            return false;
        }
    }

    return true;
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
    if (meshIndex < 0 || static_cast<uint32_t>(meshIndex) >= this->model.meshes_count) 
    {
        LogWarn("Mesh index '%d' was out of range", meshIndex);
        return;
    }

	const tg3_mesh &tMesh = this->model.meshes[meshIndex];
    const std::string meshName = TG3ToString(tMesh.name);
	LogInfo("Mesh '%s'", meshName.c_str());
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

        if (primitive.mode != -1 && primitive.mode != TG3_MODE_TRIANGLES) 
        {
            LogWarn("Primitive '%zu' on mesh '%s' uses mode '%d'; only triangle lists are supported", primIndex, meshName.c_str(), primitive.mode);
            continue;
        }
		
		std::vector<DirectX::XMFLOAT3> positions = {};
		std::vector<DirectX::XMFLOAT3> normals = {};
		std::vector<DirectX::XMFLOAT2> uvs = {};
		std::vector<uint32_t> indicies = {};

        if (!ReadPrimitiveAttributes(primitive, positions, normals, uvs) ||
            !ReadPrimitiveIndices(primitive, static_cast<uint32_t>(positions.size()), indicies)
        ) {
            LogWarn("Primitive could not be read (skipped)");
            continue;
        }

        if (normals.size() != positions.size()) 
        {
            LogWarn("Primitve has '%zu' positions but only '%zu' normals", positions.size(), normals.size());
            normals.assign(positions.size(), {0.0f, 0.0f, 1.0f});
        }

        if (uvs.size() != positions.size()) 
        {
            LogWarn("Primitve has '%zu' positions but only '%zu' UVs", positions.size(), uvs.size());
            uvs.assign(positions.size(), {0.0f, 0.0f});
        }

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

        const uint32_t baseVertex = static_cast<uint32_t>(meshVertices.size());
		
		for (size_t i = 0; i < positions.size(); ++i)
			meshVertices.emplace_back(positions[i], normals[i], uvs[i]);

		SubMesh subMesh{};
		subMesh.startIndex = static_cast<uint32_t>(meshIndicies.size());
		subMesh.numIndicies = static_cast<uint32_t>(indicies.size());

        if (primitive.material >= 0 && static_cast<uint32_t>(primitive.material) < this->model.materials_count) 
        {
            subMesh.materialIndex = primitive.material;
        }
        else 
        {
            if (primitive.material != -1)
                LogWarn("Primitive '%zu' on mesh '%s' has invalid material '%d' (using default)", primIndex, meshName.c_str(), primitive.material);

            subMesh.materialIndex = GetDefaultMaterialIndex();
        }

		subMeshes.push_back(subMesh);

		for (size_t i = indicies.size(); i-- > 0;)
			meshIndicies.push_back(indicies[i] + baseVertex);

        this->vertexCount += static_cast<uint32_t>(positions.size());
        this->indexCount += static_cast<uint32_t>(indicies.size());
        ++this->subMeshCount;
	}

    if (meshVertices.empty() || meshIndicies.empty()) 
    {
        LogWarn("Mesh '%s' has no valid primitives (skipped)", meshName.c_str());
        LogUnindent();
        return;
    }

    ComputeTangents(meshVertices, meshIndicies);

	Mesh *mesh = new Mesh();
	bool result = mesh->Create(
		meshName,
		meshVertices,
		meshIndicies,
		subMeshes,
		device
	);
    if (!result) {
        LogWarn("Failed to create mesh '%s'", meshName.c_str());

        delete mesh;

        LogUnindent();
        return;
    }

	meshes.push_back(mesh);

	LogUnindent();
}
