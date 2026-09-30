#include "../shared_resources.hlsli"

// Input
struct VertexShaderInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
};

// Output
struct VertexShaderOutput
{
    float4 clipPosition : SV_POSITION;
    float3 worldPosition : WORLD_POSITION;
    float2 uv : TEXCOORD0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    
    float4 worldPos = mul(float4(input.position.xyz, 1.0f), worldMatrix);
    float4 clipPos = mul(worldPos, viewProjectionMatrix);

    output.clipPosition = clipPos;
    output.worldPosition = worldPos.xyz;

    output.uv = input.uv;
    
    return output;
}