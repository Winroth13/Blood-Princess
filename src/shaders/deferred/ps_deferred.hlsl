#include "../shared_resources.hlsli"
#include "hsv.hlsli"

Texture2D albedoTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D metallicRoughnessTexture : register(t2);
Texture2D paintMaskTexture : register(t3);

struct PixelShaderInput
{
    float4 position         : SV_POSITION;
    float3 worldNormal      : WORLD_NORMAL;
    float2 uv0              : TEXCOORD0;
    float2 uv1              : TEXCOORD1;
    float3 tangent          : TANGENT;
    float3 bitangent        : BITANGENT;
};

struct PixelShaderOutput
{
    float4 albedoMetallic : SV_Target0;
    float4 normalRoughness : SV_Target1;
};

PixelShaderOutput main(PixelShaderInput input)
{
    PixelShaderOutput output;
    
    float4 albedoSample = albedoTexture.Sample(samplerLinearWrap, input.uv0);
    if (albedoSample.a < 0.5f)
        discard;
    
    float4 metallicRoughnessSample = metallicRoughnessTexture.Sample(samplerLinearWrap, input.uv0);
    
    float3 albedo = albedoSample.rgb * materialTint;

    if ((materialFlags & MATERIAL_FLAG_HSV_ADJUSTED) == MATERIAL_FLAG_HSV_ADJUSTED)
    {
        float3 hsvAlbedo = RgbToHsv(albedo);
        hsvAlbedo.r = lerp(hsvAlbedo.r, hsvShift.r, hsvShift.a);
        hsvAlbedo.g *= hsvShift.g;
        hsvAlbedo.b *= hsvShift.b;
        albedo = HsvToRgb(hsvAlbedo);
    }
    
    float3 normalSample = normalTexture.Sample(samplerLinearWrap, input.uv0).rgb * 2.0f - 1.0f;
    float3x3 tbnMatrix = float3x3(
        normalize(input.tangent),
        normalize(input.bitangent),
        normalize(input.worldNormal)
    );
    float3 normal = normalize(mul(normalSample, tbnMatrix)) * 0.5f + 0.5f;
    
    float4 paintMaskSample = paintMaskTexture.SampleLevel(samplerLinearClamp, input.uv1, 0);
    
    float3 finalAlbedo = lerp(albedo, paintMaskSample.rgb, paintMaskSample.a);
    float finalRoughness = lerp(metallicRoughnessSample.g, 0.62f, paintMaskSample.a);
    
    output.albedoMetallic = float4(finalAlbedo, metallicRoughnessSample.b);
    output.normalRoughness = float4(normal, finalRoughness);
    
    return output;
}