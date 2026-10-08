#include "../shared_resources.hlsli"

struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float3 worldPosition : WORLD_POSITION;
    float2 uv : TEXCOORD0;
};

Texture2D albedoTexture : register(t0);

void main(PixelShaderInput input)
{
    float4 albedoSample = albedoTexture.Sample(samplerLinearWrap, input.uv);
    if (albedoSample.a < 0.5f)
        discard;
	
	return;
}