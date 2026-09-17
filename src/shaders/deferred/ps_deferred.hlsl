#include "../shared_resources.hlsli"

Texture2D albedoTexture : register(t0);
Texture2D normalTexture : register(t1);

struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float3 worldPosition : WORLD_POSITION;
    float3 worldNormal : WORLD_NORMAL;
    float2 uv : UV;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
};

struct PixelShaderOutput
{
    float4 albedoSpecularExponent : SV_Target0;
    float4 normalSpecular : SV_Target1;
};

PixelShaderOutput main(PixelShaderInput input)
{
    PixelShaderOutput output;
    
    float4 albedoSample = albedoTexture.Sample(samplerLinearWrap, input.uv);
    if (albedoSample.a < 0.5f)
        discard;
    
    float3 albedo = albedoSample.rgb * materialTint;
    
    //float3 normalSample = normalTexture.Sample(samplerLinearWrap, input.uv).rgb * 2.0f - 1.0f; 
    //float3x3 tbnMatrix = float3x3(
    //    normalize(input.tangent),
    //    normalize(input.bitangent),
    //    normalize(input.worldNormal)
    //);
    //float3 normal = normalize(mul(normalSample, tbnMatrix)) * 0.5f + 0.5f;
    
    float3 normal = input.worldNormal * 0.5f + 0.5f;
     
    output.albedoSpecularExponent = float4(albedo, materialSpecular / 1000.0f);
    output.normalSpecular = float4(normal, materialSpecularExponent);
    
    return output;
}