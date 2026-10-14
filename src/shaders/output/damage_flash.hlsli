#ifndef DAMAGE_FLASH
#define DAMAGE_FLASH

#include "../shared_resources.hlsli"

float4 DamageFlash(float2 uv)
{
    float2 borderUV = uv * 2.0 - 1.0;

    float dist = max(abs(borderUV.x), abs(borderUV.y));

    float a = smoothstep(damageFlashInnerRatio, damageFlashInnerRatio + damageFlashFeather, dist);
    a = pow(a, damageFlashEdgePower);
    a *= damageFlashIntensity;

    return float4(damageFlashColour.rgb, a * damageFlashColour.a);
}

#endif // DAMAGE_FLASH