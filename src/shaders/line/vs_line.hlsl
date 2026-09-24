#include "../shared_resources.hlsli"

struct VertexShaderInput
{
    float3 worldPosition : POSITION;
    float3 color : COLOR;
};

struct VertexShaderOutput
{
    float4 clipPosition : SV_POSITION;
    float3 color : COLOR;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    float4 clipPos = mul(float4(input.worldPosition, 1.0f), viewProjectionMatrix);

    output.clipPosition = clipPos;
    output.color = input.color;
    
    return output;
}