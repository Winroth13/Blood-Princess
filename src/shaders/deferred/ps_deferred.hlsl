#include "../shared_resources.hlsli"

Texture2D albedoTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D metallicRoughnessTexture : register(t2);

struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float3 worldNormal : WORLD_NORMAL;
    float2 uv : UV;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
};

struct PixelShaderOutput
{
    float4 albedoMetallic : SV_Target0;
    float4 normalRoughness : SV_Target1;
};

PixelShaderOutput main(PixelShaderInput input)
{
    PixelShaderOutput output;
    
    float4 albedoSample = albedoTexture.Sample(samplerLinearWrap, input.uv);
    if (albedoSample.a < 0.5f)
        discard;
    
    float4 metallicRoughnessSample = metallicRoughnessTexture.Sample(samplerLinearWrap, input.uv);
    
    float3 albedo = albedoSample.rgb * materialTint;
    
    float3 normalSample = normalTexture.Sample(samplerLinearWrap, input.uv).rgb * 2.0f - 1.0f; 
    float3x3 tbnMatrix = float3x3(
        normalize(input.tangent),
        normalize(input.bitangent),
        normalize(input.worldNormal)
    );
    float3 normal = normalize(mul(normalSample, tbnMatrix)) * 0.5f + 0.5f;
    
    output.albedoMetallic = float4(albedo, metallicRoughnessSample.b);
    output.normalRoughness = float4(normal, metallicRoughnessSample.g);
    
    return output;
}