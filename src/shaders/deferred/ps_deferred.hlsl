#include "../shared_buffers.hlsli"

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
    float4 position : SV_Target0;
    float4 normal : SV_Target1;
    float4 color : SV_Target2;
};


PixelShaderOutput main(PixelShaderInput input)
{
    PixelShaderOutput output;
    output.color = float4(0.0f, 1.0f, 0.0f, 1.0f);
    
    return output;
}