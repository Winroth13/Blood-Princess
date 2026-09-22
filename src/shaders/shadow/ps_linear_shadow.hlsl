#include "../shared_resources.hlsli"

struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

Texture2D albedoTexture : register(t0);

float4 main() : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}