#include "../shared_buffers.hlsli"

// Input
struct VertexShaderInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
    float3 tangent : TANGENT;
};

// Output
struct VertexShaderOutput
{
    float4 clipPosition : SV_POSITION;
    float3 worldPosition : WORLD_POSITION;
    float3 worldNormal : WORLD_NORMAL;
    float2 uv : UV;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    
    float4 worldPos = mul(float4(input.position.xyz, 1.0f), worldMatrix);
    float4 clipPos = mul(worldPos, viewProjectionMatrix);

    output.clipPosition = clipPos;
    output.worldPosition = worldPos.xyz;

    output.worldNormal = normalize(mul(input.normal, (float3x3) worldInvTransposeMatrix));
    output.uv = input.uv;
    
    float3 tangent = input.tangent;
    float3 normal = normalize(input.normal);
    
    output.tangent = normalize(tangent - dot(tangent, normal) * normal);
    output.bitangent = cross(normal, output.tangent);
    
    return output;
}