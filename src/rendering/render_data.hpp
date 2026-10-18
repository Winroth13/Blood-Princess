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
    float totalTime;
    float deltaTime;
    uint32_t ticks;
    float   pad0;
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
    uint32_t materialFlags;
    float	 pad0[2];
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

constexpr uint32_t SKINNING_MAX_JOINT_COUNT = 256;

struct SkinningData {
    XMFLOAT4X4 skinMatrices[SKINNING_MAX_JOINT_COUNT];
};
AssertCBufferSize(SkinningData);

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
    XMFLOAT3 velocity;
    XMFLOAT3 tint;
    float scale;
    float lifeTime;
};

struct EnvironmentData
{
    XMFLOAT4 hsvShift;
};
AssertCBufferSize(EnvironmentData);

struct PaintData
{
    XMFLOAT3 paintPosition;
    float paintRadius;
};
AssertCBufferSize(PaintData);