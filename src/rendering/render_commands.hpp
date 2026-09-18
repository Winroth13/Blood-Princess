#pragma once

#include "assets/types/model.hpp"

#include <DirectXMath.h>
#include <d3d11.h>

struct Material;

using namespace DirectX;

struct GeometryCommand 
{	
    ID3D11Buffer *vertexBuffer;
    ID3D11Buffer *indexBuffer;

    uint32_t startIndex;
    uint32_t indexCount;

    Material material;
    DirectX::XMFLOAT3 tint;

    DirectX::XMFLOAT4X4 worldMatrix;

    ID3D11ShaderResourceView *albedoSRV;
    ID3D11ShaderResourceView *normalSRV;
    ID3D11ShaderResourceView *metallicRoughnessSRV;
};

struct DirectionalLightCommand 
{
    XMFLOAT3 direction;
    XMFLOAT3 color;
    float intensity;
};

struct PointLightCommand 
{
    XMFLOAT3 position;
    XMFLOAT3 color;
    float attenuation;
    float radius;
    float intensity;
};

struct SpotLightCommand 
{
    XMFLOAT3 position;
    XMFLOAT3 direction;
    XMFLOAT3 color;
    float range;
    float attenuation;
    float intensity;
    float innerAngleCos;
    float outerAngleCos;
};

struct SkyboxCommand 
{
	
};