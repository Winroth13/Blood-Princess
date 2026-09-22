SamplerState samplerLinearWrap : register(s0);

cbuffer PerView : register(b0)
{
    float4x4 viewMatrix;
    float4x4 invViewMatrix;
    float4x4 projectionMatrix;
    float4x4 invProjectionMatrix;
    float4x4 viewProjectionMatrix;
    float4x4 invViewProjectionMatrix;
    float3 cameraPosition;
    float farPlane;
    float nearPlane;
    float3 pad0;
}

cbuffer PerObject : register(b1)
{
    float4x4 worldMatrix;
    float4x4 worldInvTransposeMatrix;
};

cbuffer PerMaterial : register(b2)
{
    float3 materialTint;
    float materialSpecular;
    float materialSpecularExponent;
    float3 pad1;
}

cbuffer Light : register(b3)
{
    float3 ambientColour;
    int directionalLightCount;
    int spotLightCount;
    int pointLightCount;
    float2 pad2;
}

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