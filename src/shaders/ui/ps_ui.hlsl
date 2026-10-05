#include "../shared_resources.hlsli"

Texture2D uiTexture : register(t0);

struct PixelShaderInput
{
    float4 clipPosition : SV_POSITION;
    float4 colour : COLOUR;
    float2 uv : TEXCOORD;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    float4 textureSample = uiTexture.Sample(samplerLinearWrap, input.uv);
    
    float4 finalColour = textureSample * input.colour;

    return finalColour;
}