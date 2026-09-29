#include "../shared_resources.hlsli"

struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float3 worldPosition : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
    float3 normal : NORMAL;
};

cbuffer Paint : register(b7)
{
    float3 paintPosition;
    float paintRadius;
};

Texture2D splatterMaskTexture : register(t0);

float4 main(PixelShaderInput input) : SV_TARGET
{
    float3 delta = input.worldPosition - paintPosition;
    float dist = length(delta);
    
    float4 paintColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    
    float3 normal = normalize(input.normal);
    float3 up = float3(0.0f, 1.0f, 0.0f);
    float3 tangent = normalize(cross(up, normal));
    float3 bitangent = cross(normal, tangent);

    float2 localPosition = float2(dot(delta, tangent), dot(delta, bitangent));
    
    if (abs(localPosition.x) > paintRadius || abs(localPosition.y) > paintRadius)
        discard;
    
    float2 localUV = (localPosition / paintRadius) * 0.5f + 0.5f;
    float4 maskSample = splatterMaskTexture.Sample(samplerLinearWrap, localUV);

    float4 finalColor = paintColor * maskSample;
    finalColor.rgb *= finalColor.a;
    
    return finalColor;

}