#include "../shared_resources.hlsli"

// Input
struct VertexShaderInput
{
    float2 position : POSITION;
    float2 uv : TEXCOORD;
    float4 colour : COLOR;
};

// Output
struct VertexShaderOutput
{
    float4 clipPosition : SV_POSITION;
    float4 colour : COLOUR;
    float2 uv : TEXCOORD;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;

    output.clipPosition = mul(float4(input.position, 1.0f, 1.0f), viewProjectionMatrix);;
    output.colour = input.colour;
    output.uv = input.uv;
    
    return output;
}