#include "../shared_resources.hlsli"

// Input
struct VertexShaderInput
{
    float2 position : POSITION;
    float2 uv : TEXCOORD;
    float4 colour : COLOR;
    uint isText : IS_TEXT;
    float4 outlineColour : OUTLINE_COLOUR;
    float outlineWidth : OUTLINE_WIDTH;
    float distanceScale : DISTANCE_SCALE;
};

// Output
struct VertexShaderOutput
{
    float4 clipPosition : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 colour : COLOUR;
    nointerpolation uint isText : IS_TEXT;
    float4 outlineColour : OUTLINE_COLOUR;
    float outlineWidth : OUTLINE_WIDTH;
    float distanceScale : DISTANCE_SCALE;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;

    output.clipPosition = mul(float4(input.position, 1.0f, 1.0f), viewProjectionMatrix);;
    output.colour = input.colour;
    output.uv = input.uv;
    output.isText = input.isText;
    output.outlineColour = input.outlineColour;
    output.outlineWidth = input.outlineWidth;
    output.distanceScale = input.distanceScale;
    
    return output;
}