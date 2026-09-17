#include "../shared_resources.hlsli"

RWTexture2DArray<unorm float4> backBufferUAV;

Texture2D<float4> positionGBuffer : register(t11);
Texture2D<float4> normalGBuffer : register(t12);
Texture2D<float4> colorGBuffer : register(t13);

struct DirectionalLight
{
    float4x4 viewProjMatrix;
    float3 direction;
    float intensity;
    float3 color;
    float pad0;
};

struct PointLight
{
    float3 position;
    float attenuation;
    float3 color;
    float intensity;
};

struct SpotLight
{
    float4x4 viewProjMatrix;
    float3 position;
    float attenuation;
    float3 color;
    float intensity;
    float3 direction;
    float angle;
};

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{

}