SamplerState samplerLinearWrap : register(s0);

cbuffer PerFrame : register(b0)
{
    float deltaTime;
    uint ticks;
    float2 pad0;
};

cbuffer PerView : register(b1)
{
    float4x4 viewMatrix;
    float4x4 invViewMatrix;
    float4x4 projectionMatrix;
    float4x4 invProjectionMatrix;
    float4x4 viewProjectionMatrix;
    float4x4 invViewProjectionMatrix;
    float3 cameraPosition;
    float nearPlane;
    float farPlane;
    float3 pad1;
}

cbuffer PerObject : register(b2)
{
    float4x4 worldMatrix;
    float4x4 worldInvTransposeMatrix;
};

cbuffer PerMaterial : register(b3)
{
    float3 materialTint;
    float materialSpecular;
    float materialSpecularExponent;
    float3 pad2;
}

cbuffer Light : register(b4)
{
    float3 ambientColour;
    int directionalLightCount;
    int spotLightCount;
    int pointLightCount;
    float2 pad3;
}

#define PARTICLE_FLAG_ANIMATED 1 << 0
#define PARTICLE_FLAG_ADDITIVE 1 << 1
#define PARTICLE_FLAG_RESET 1 << 2

cbuffer ParticleSystem : register(b5)
{
    float       lifeTime;
    float       spawnRadius;
    float2      atlasSize;
    float3      velocity;
    float       animationSpeed;
    float3      acceleration;
    float       startScale;
    float3      startTint;
    float       endScale;
    float3      endTint;
    int         count;
    uint        flags;
    float3      pad4;
};

struct DirectionalLight
{
    float3 direction;
    int castShadows;
    float3 colour;
    float4x4 viewProjectionMatrix;
    int shadowArrayIndex;
};

struct PointLight
{
    float3 position;
    float pad0;
    float3 colour;
    float radius;
    int castShadows;
    int shadowArrayIndex;
    float2 pad1;
};

struct SpotLight
{
    float3 position;
    float range;
    float3 direction;
    float innerAngleCos;
    float3 colour;
    float outerAngleCos;
    int castShadows;
    float4x4 viewProjectionMatrix;
    int shadowArrayIndex;
    float2 pad0;
};

struct Particle
{
    float3 position;
    float3 tint;
    float scale;
    float lifeTime;
};