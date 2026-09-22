#include "../shared_resources.hlsli"

// Input
struct VertexShaderInput
{
    float3 position : POSITION;
    float2 uv : UV;
};

// Output
struct VertexShaderOutput
{
    float4 clipPosition : SV_POSITION;
    float2 uv : UV;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    
    float4 worldPos = mul(float4(input.position.xyz, 1.0f), worldMatrix);
    float4 clipPos = mul(worldPos, viewProjectionMatrix);

    output.clipPosition = clipPos;

    output.uv = input.uv;
    
    return output;
}