#pragma once
#include <DirectXMath.h>

using namespace DirectX;

#define AssertCBufferSize(type) static_assert(sizeof(type) % 16 == 0)

//struct PerFrameBuffer
//{
//	DirectX::XMFLOAT3 ambientColor;
//	uint32_t numDirectionalLights;
//	uint32_t numPointLights;
//	uint32_t numSpotLights;
//	uint32_t flags;
//	float pad0;
//	std::array<uint32_t, 2> screenDimensions;
//	DirectX::XMFLOAT2 pad1;
//};

struct PerFrameData
{
    float deltaTime;
    uint32_t ticks;
    XMFLOAT2   pad0;
};
AssertCBufferSize(PerFrameData);

struct PerViewData
{
    XMFLOAT4X4 viewMatrix;
    XMFLOAT4X4 invViewMatrix;
    XMFLOAT4X4 projectionMatrix;
    XMFLOAT4X4 invProjectionMatrix;
    XMFLOAT4X4 viewProjectionMatrix;
    XMFLOAT4X4 invViewProjectionMatrix;
    XMFLOAT3   cameraPosition;
    float      nearPlane;
    float      farPlane;
    XMFLOAT3   pad0;
};
AssertCBufferSize(PerViewData);

struct PerObjectData
{
    XMFLOAT4X4 worldMatrix;
    XMFLOAT4X4 worldInverseTransposeMatrix;
};
AssertCBufferSize(PerObjectData);

struct PerMaterialData
{
    XMFLOAT3 materialTint;
    float    materialSpecular;
    float    materialSpecularExponent;
    float	 pad0[3];
};
AssertCBufferSize(PerMaterialData);

struct LightData
{
    XMFLOAT3 ambientColour;
    int directionalLightCount;
    int spotLightCount;
    int pointLightCount;
    float pad0[2];
};
AssertCBufferSize(LightData);

struct DirectionalLightData 
{
    XMFLOAT3   direction;
    int        castShadows;
    XMFLOAT3   color;
    XMFLOAT4X4 viewProjectionMatrix;
    int        shadowArrayIndex;
};
AssertCBufferSize(DirectionalLightData);

struct PointLightData
{
    XMFLOAT3    position;
    float       pad0;
    XMFLOAT3    color;
    float       radius;
    int         castShadows;
    int         shadowArrayIndex;
    float       pad1[2];
};
AssertCBufferSize(PointLightData);

struct SpotLightData
{
    XMFLOAT3   position;
    float      range;
    XMFLOAT3   direction;
    float      innerAngleCos;
    XMFLOAT3   color;
    float      outerAngleCos;
    int        castShadows;
    XMFLOAT4X4 viewProjectionMatrix;
    int        shadowArrayIndex;
    float      pad0[2];
};
AssertCBufferSize(SpotLightData);

struct ParticleSystemData
{
    float lifeTime;
    float spawnRadius;
    XMFLOAT2 atlasSize;

    XMFLOAT3 velocity;
    float animationSpeed;

    XMFLOAT3 acceleration;
    float startScale;

    XMFLOAT3 startTint;
    float endScale;

    XMFLOAT3 endTint;
    int count;
    uint32_t flags; // particles should reset automatically
    float pad0[3];
};
AssertCBufferSize(ParticleSystemData);

struct ParticleData
{
    XMFLOAT3 position;
    XMFLOAT3 tint;
    float scale;
    float lifeTime;
};
AssertCBufferSize(ParticleData);

struct PaintData
{
    XMFLOAT3 paintPosition;
    float pad0;
};
AssertCBufferSize(PaintData);