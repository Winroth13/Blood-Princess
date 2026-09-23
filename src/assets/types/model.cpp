#include "assets/types/model.hpp"
#include "assets/types/texture_2d.hpp"
#include "core/logging.hpp"
#include "imgui/imgui.h"

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
	if (this->vertexBuffer)
	{
		this->vertexBuffer->Release();
	}

	if (this->indexBuffer)
	{
		this->indexBuffer->Release();
	}
}

bool Mesh::Create(
	const std::string &name,
	const std::vector<Vertex> &vertices,
	const std::vector<uint32_t> &indicies,
	const std::vector<SubMesh> &subMeshes,
	ID3D11Device *device
)
{
	/* 
	* TODO: Logging all this becomes VERY verbose when creating models.
	* We either need to add verbosity levels to the logging or just not
	* log here.
	*/
	//LogInfo("Creating Mesh...");
	//LogIndent();

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
		//LogInfo("Created Vertex Buffer");
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
		//LogInfo("Created Index Buffer");
	}

	this->numVertices = vertices.size();
	this->numIndicies = indicies.size();
	this->subMeshes = subMeshes;
	this->name = name;

	//LogUnindent();

	return true;
}
#pragma endregion

bool Model::Create(
	const std::vector<Mesh*> &meshes, 
	const std::vector<Material> &materials,
	const std::vector<std::shared_ptr<Texture2D>> &textures
)
{
	LogInfo("Creating Model...");
	LogIndent();

	for (auto &mesh : meshes)
	{
		this->meshes.push_back(std::unique_ptr<Mesh>(mesh));
	}

	this->materials = materials;	
	this->textures = textures;
	
	LogUnindent();

	return true;
}

Model::~Model()
{
	LogInfo("Destroying Model '%s'", this->path.c_str());
}

Texture2D *Model::GetTexture(int32_t index)
{
	if (index < 0 || index >= this->textures.size()) {
		LogWarn("Texture with index: %d doesn't exist on material", index);
		return nullptr;
	}

	return this->textures[index].get();
}

void Model::DrawImGui()
{
	if (ImGui::TreeNode("Meshes"))
	{
		int meshIndex = 0;

		for (auto &mesh : this->meshes)
		{
			std::string label = std::to_string(meshIndex);
			if (ImGui::TreeNode(label.c_str()))
			{
				ImGui::Text("Vertices: %d", mesh->GetNumVertices());
				ImGui::Text("Indices: %d", mesh->GetNumIndicies());

				if (ImGui::TreeNode("Submeshes"))
				{
					int subMeshIndex = 0;

					for (auto &submesh : mesh->GetSubMeshes())
					{
						if (ImGui::TreeNode(std::to_string(subMeshIndex).c_str()))
						{
							ImGui::Text("Start Index: %d", submesh.startIndex);
							ImGui::Text("Index Count: %d", submesh.numIndicies);
							ImGui::TreePop();
						}
						subMeshIndex++;
					}
					ImGui::TreePop();
				}

				ImGui::TreePop();
			}
			meshIndex++;
		}

		ImGui::TreePop();
	}
}