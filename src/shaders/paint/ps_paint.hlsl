#include "../shared_resources.hlsli"

struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float3 worldPosition : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
};

cbuffer Paint : register(b7)
{
    float3 paintPosition;
    float Paint_pad0;
};

Texture2D splatterMaskTexture : register(t0);

float4 main(PixelShaderInput input) : SV_TARGET
{
    float dist = distance(input.worldPosition, paintPosition);
    
    float paintRadius = 1.0f;
    float paintHardness = 0.35f;
    float4 paintColor = float4(1.0f, 0.0f, 0.0f, 1.0f);
    
    // Signed Distance Field / Radial Falloff calculation
    float edge = 1.0f - saturate((dist - (paintRadius * paintHardness)) / (paintRadius * (1.0f - paintHardness)));
    
    if (edge <= 0.0f)
        discard;

    return float4(paintColor.rgb, edge * paintColor.a);
}