#include "mesh.hpp"
#include "core/logging.hpp"

#include <d3d11.h>

#pragma region Vertex

bool operator== (const Vertex &v1, const Vertex &v2)
{
	return
		(
			v1.position.x == v2.position.x &&
			v1.position.y == v2.position.y &&
			v1.position.z == v2.position.z &&

			v1.normal.x == v2.normal.x &&
			v1.normal.y == v2.normal.y &&
			v1.normal.z == v2.normal.z &&

			v1.uv.x == v2.uv.x &&
			v1.uv.y == v2.uv.y
			);
}

#pragma endregion Vertex

#pragma region Mesh
Mesh::~Mesh()
{
	LogInfo("Destroying Mesh...");
	LogIndent();

	LogInfo("Releasing Vertex Buffer");
	if (this->vertexBuffer)
	{
		this->vertexBuffer->Release();
	}

	LogInfo("Releasing Index Buffer");
	if (this->indexBuffer)
	{
		this->indexBuffer->Release();
	}

	LogUnindent();
}

bool Mesh::Create(
	const std::vector<Vertex> &vertices, 
	const std::vector<uint32_t> &indicies,
	ID3D11Device *device
)
{
	LogInfo("Creating Mesh...");
	LogIndent();

	HRESULT hr;

	/* Create Vertex Buffer */
	{
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = UINT(sizeof(Vertex) * vertices.size());
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA data = {};
		data.pSysMem = vertices.data();
		data.SysMemPitch = 0;
		data.SysMemSlicePitch = 0;

		hr = device->CreateBuffer(&desc, &data, &this->vertexBuffer);

		if (FAILED(hr))
		{
			LogError("Failed to create Vertex Buffer");
			return false;
		}
		LogInfo("Created Vertex Buffer");
	}

	/* Create Index Buffer */
	{
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = UINT(sizeof(uint32_t) * indicies.size());
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA data = {};
		data.pSysMem = indicies.data();
		data.SysMemPitch = 0;
		data.SysMemSlicePitch = 0;

		hr = device->CreateBuffer(&desc, &data, &this->indexBuffer);

		if (FAILED(hr))
		{
			LogError("Failed to create Index Buffer");
			return false;
		}
		LogInfo("Created Index Buffer");
	}

	this->numVertices = vertices.size();
	this->numIndicies = indicies.size();

	LogUnindent();

	return true;
}
#pragma endregion
