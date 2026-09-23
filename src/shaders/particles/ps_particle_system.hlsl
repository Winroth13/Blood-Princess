#include "../shared_resources.hlsli"

struct PixelShaderInput
{
    float4 clipPosition : SV_POSITION;
    float3 tint : TINT;
    float lifetime : LIFETIME;
    float2 uv : UV;
};

Texture2D colorTexture : register(t0);

SamplerState samplerState : register(s0);

float2 AtlasUV(float2 uv, float ratio)
{
    uint numTextures = uint(atlasSize.x * atlasSize.y);
    
    uint atlasIndex = uint((numTextures - 1) * ratio * animationSpeed) % numTextures;
    
    uint atlasX = atlasIndex % (uint) atlasSize.x;
    uint atlasY = atlasIndex / (uint) atlasSize.y;
    
    return float2(
        (uv.x + atlasX) / atlasSize.x,
        (uv.y + atlasY) / atlasSize.y
    );
}

float4 main(PixelShaderInput input) : SV_TARGET
{
    float2 uv;
    float lifeRatio = input.lifetime / lifeTime;
    
    if ((flags & PARTICLE_FLAG_ANIMATED) == PARTICLE_FLAG_ANIMATED)
    {
        uv = AtlasUV(input.uv, lifeRatio);
    }
    else
    {
        uv = input.uv;
    }

    float4 colorSample = colorTexture.Sample(samplerState, uv);
    
    /* Alpha check */
    if (colorSample.a < 0.5f)
    {
        discard;
    }
    
    /* Diffuse */
    float3 color = colorSample.rgb * input.tint;
    
    return float4(color, 1.0f);
}