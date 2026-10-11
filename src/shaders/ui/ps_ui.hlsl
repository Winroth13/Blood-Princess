#include "../shared_resources.hlsli"

Texture2D uiTexture : register(t0);

#define SDF_EDGE (128.0f / 255.0f)

struct PixelShaderInput
{
    float4 clipPosition : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 colour : COLOUR;
    nointerpolation uint isText : IS_TEXT;
    float4 outlineColour : OUTLINE_COLOUR;
    float outlineWidth : OUTLINE_WIDTH;
    float distanceScale : DISTANCE_SCALE;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    if (!input.isText)
        return uiTexture.Sample(samplerLinearClamp, input.uv) * input.colour;

    float field = uiTexture.Sample(samplerLinearClamp, input.uv).r;
    float distance = (field - SDF_EDGE) * input.distanceScale;

    float fillCoverage = saturate(distance + 0.5f);
    float outlineCoverage = saturate(distance + input.outlineWidth + 0.5f);

    float fillAlpha = input.colour.a * fillCoverage;
    float outlineAlpha = input.outlineColour.a * outlineCoverage * (1.0f - fillAlpha);
    float alpha = fillAlpha + outlineAlpha;

    float3 colour = (input.colour.rgb * fillAlpha + input.outlineColour.rgb * outlineAlpha) / max(alpha, 1e-5f);
    return float4(colour, alpha);
}