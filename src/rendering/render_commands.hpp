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

struct AmbientLightCommand 
{
    XMFLOAT3 colour;
    float intensity;
};

struct DirectionalLightCommand 
{
    XMFLOAT3 direction;
    XMFLOAT3 colour;
    float intensity;
    bool castShadows;
    XMFLOAT4X4 viewProjectionMatrix;
};

struct PointLightCommand 
{
    XMFLOAT3 position;
    XMFLOAT3 colour;
    float radius;
    float intensity;
    bool castShadows;
    XMFLOAT4X4 viewProjectionMatrix;
};

struct SpotLightCommand 
{
    XMFLOAT3 position;
    XMFLOAT3 direction;
    XMFLOAT3 colour;
    float range;
    float intensity;
    float innerAngle; // radians
    float outerAngle; // radians
    bool castShadows;
    XMFLOAT4X4 viewProjectionMatrix;
};

struct SkyboxCommand 
{
	
};