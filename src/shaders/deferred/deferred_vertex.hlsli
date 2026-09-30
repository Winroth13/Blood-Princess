#ifndef DEFERRED_VERTEX_HLSLI
#define DEFERRED_VERTEX_HLSLI

#include "../shared_resources.hlsli"

struct VertexShaderOutput
{
    float4 clipPosition : SV_POSITION;
    float3 worldNormal : WORLD_NORMAL;
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
};

VertexShaderOutput TransformVertex(float3 position, float3 normal, float2 uv0, float2 uv1, float4 tangent)
{
    VertexShaderOutput output;
    
    float4 worldPosition = mul(float4(position.xyz, 1.0f), worldMatrix);
    output.clipPosition = mul(worldPosition, viewProjectionMatrix);
    
    float3 worldNormal = normalize(mul(normal, (float3x3) worldInvTransposeMatrix));
    float3 worldTangent = normalize(mul(tangent.xyz, (float3x3) worldMatrix));
    
    worldTangent = normalize(worldTangent - dot(worldTangent, worldNormal) * worldNormal);
    
    output.worldNormal = worldNormal;
    output.uv0 = uv0;
    output.uv1 = uv1;
    output.tangent = worldTangent;
    output.bitangent = cross(worldNormal, worldTangent) * tangent.w;
    
    return output;
}

#endif