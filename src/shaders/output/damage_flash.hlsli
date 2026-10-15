#ifndef DAMAGE_FLASH
#define DAMAGE_FLASH

#include "../shared_resources.hlsli"

float4 DamageFlash(Texture2D<float4> noiseTexture, float2 uv)
{
    float2 borderUV = uv * 2.0 - 1.0;

    float dist = max(abs(borderUV.x), abs(borderUV.y));

    float a = smoothstep(damageFlashInnerRatio, damageFlashInnerRatio + damageFlashFeather, dist);
    a = pow(a, damageFlashEdgePower);
    a *= damageFlashIntensity;
    
    float2 toCentre = float2(0.5f, 0.5f) - uv;
    float noise = noiseTexture.SampleLevel(samplerLinearWrap, (uv + toCentre * totalTime) * 0.05f, 0).r;

    return float4(damageFlashColour.rgb, a * damageFlashColour.a * noise * noise * 2);
}

#endif // DAMAGE_FLASH