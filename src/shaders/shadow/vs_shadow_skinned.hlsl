#include "../shared_resources.hlsli"
#include "../skinning.hlsli"

// Input
struct VertexShaderInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
    
    uint4 joints : JOINTS;
    float4 weights : WEIGHTS;
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
    
    float3 position = mul(float4(input.position, 1.0f), GetSkinMatrix(input.joints, input.weights)).xyz;
    float4 worldPosition = mul(float4(position, 1.0f), worldMatrix);
    
    output.clipPosition = mul(worldPosition, viewProjectionMatrix);
    output.worldPosition = worldPosition.xyz;

    output.uv = input.uv;
    
    return output;
}