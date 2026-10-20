#pragma once

#include "assets/types/model.hpp"
#include "rendering/lines/line_vertex.hpp"

#include <vector>
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
    uint32_t materialFlags;

    DirectX::XMFLOAT4X4 worldMatrix;

    ID3D11ShaderResourceView *albedoSRV;
    ID3D11ShaderResourceView *normalSRV;
    ID3D11ShaderResourceView *metallicRoughnessSRV;

    ID3D11ShaderResourceView *paintMaskSRV;

    ID3D11Buffer *skinVertexBuffer;
    const DirectX::XMFLOAT4X4 *skinMatrices;
    uint32_t skinMatrixCount;
};

struct LineCommand
{
    std::vector<LineVertex> points;
    bool enableDepth;
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
    ID3D11ShaderResourceView *skyboxSRV;
};

struct EnvironmentCommand
{
    XMFLOAT4 hsvShift;
    uint32_t environmentFlags;
    float fogStartDepth;
    float fogEndDepth;
};

struct ParticleSystemCommand
{
    XMFLOAT4X4 worldMatrix;
    int count;
    uint32_t flags;
    XMFLOAT3 position;

    float lifeTime;
    float spawnRadius;

    XMFLOAT2 atlasSize;
    float animationSpeed;

    XMFLOAT3 velocity;
    XMFLOAT3 acceleration;

    float startScale;
    float endScale;

    XMFLOAT3 startTint;
    XMFLOAT3 endTint;

    ID3D11ShaderResourceView *colorSRV;
    ID3D11ShaderResourceView *particleSRV;
    ID3D11UnorderedAccessView *particleUAV;
};

struct PaintCommand
{
    ID3D11Buffer *vertexBuffer;
    ID3D11Buffer *indexBuffer;
    uint32_t startIndex;
    uint32_t indexCount;
    XMFLOAT4X4 worldMatrix;

    XMFLOAT3 paintPosition;
    float paintRadius;
    uint32_t maskWidth;
    uint32_t maskHeight;
    ID3D11RenderTargetView *paintMaskRTV;
    ID3D11ShaderResourceView *paintMaskSRV;
    ID3D11ShaderResourceView *splatterMaskSRV;
};