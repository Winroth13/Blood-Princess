#pragma once

#include "assets/types/mesh.hpp"
#include <DirectXMath.h>
#include <memory>
#include <vector>

class Texture2D;

class Scene
{
private:
	std::shared_ptr<Mesh> mesh = nullptr;
	DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixIdentity();
	float rotationX = 0.0f;
	float rotationY = 0.0f;
	std::shared_ptr<Texture2D> texture = nullptr;

public:
	bool Initialize(ID3D11Device *device)
	{
		std::vector<Vertex> vertices =
		{
			// Front Face (Z = -0.5f, Normal: 0, 0, -1)
			Vertex({ -0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f }),
			Vertex({  0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f }),
			Vertex({  0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f }),
			Vertex({ -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f }),

			// Back Face (Z = 0.5f, Normal: 0, 0, 1)
			Vertex({  0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }),
			Vertex({ -0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f }),
			Vertex({ -0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }),
			Vertex({  0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f }),

			// Top Face (Y = 0.5f, Normal: 0, 1, 0)
			Vertex({ -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f }),
			Vertex({  0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f }),
			Vertex({  0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f }),
			Vertex({ -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f }),

			// Bottom Face (Y = -0.5f, Normal: 0, -1, 0)
			Vertex({ -0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f }),
			Vertex({  0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f }),
			Vertex({  0.5f, -0.5f,  0.5f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f }),
			Vertex({ -0.5f, -0.5f,  0.5f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f }),

			// Left Face (X = -0.5f, Normal: -1, 0, 0)
			Vertex({ -0.5f,  0.5f,  0.5f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }),
			Vertex({ -0.5f,  0.5f, -0.5f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f }),
			Vertex({ -0.5f, -0.5f, -0.5f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }),
			Vertex({ -0.5f, -0.5f,  0.5f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f }),

			// Right Face (X = 0.5f, Normal: 1, 0, 0)
			Vertex({  0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }),
			Vertex({  0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f }),
			Vertex({  0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }),
			Vertex({  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f })
		};

		std::vector<uint32_t> indices =
		{
			0, 1, 2, 0, 2, 3,       // Front
			4, 5, 6, 4, 6, 7,       // Back
			8, 9, 10, 8, 10, 11,    // Top
			12, 13, 14, 12, 14, 15, // Bottom
			16, 17, 18, 16, 18, 19, // Left
			20, 21, 22, 20, 22, 23  // Right
		};

		this->mesh = std::make_shared<Mesh>();
		return this->mesh->Create(vertices, indices, device);
	}

	void Shutdown()
	{
		this->mesh = nullptr;
	}

	void Update(float deltaTime)
	{
		this->rotationX += DirectX::XMConvertToRadians(25.0f) * deltaTime;
		this->rotationY += DirectX::XMConvertToRadians(40.0f) * deltaTime;

		if (this->rotationX > DirectX::XM_2PI) this->rotationX -= DirectX::XM_2PI;
		if (this->rotationY > DirectX::XM_2PI) this->rotationY -= DirectX::XM_2PI;

		DirectX::XMMATRIX rotX = DirectX::XMMatrixRotationX(this->rotationX);
		DirectX::XMMATRIX rotY = DirectX::XMMatrixRotationY(this->rotationY);
		this->worldMatrix = rotX * rotY;
	}

	void SetMesh(std::shared_ptr<Mesh> mesh) { this->mesh = mesh; }
	std::shared_ptr<Mesh> GetMesh() const { return this->mesh; }

	void SetTexture(std::shared_ptr<Texture2D> texture) { this->texture = texture; }
	std::shared_ptr<Texture2D> GetTexture() const { return this->texture; }

	DirectX::XMMATRIX GetWorldMatrix() const { return this->worldMatrix; }
};
