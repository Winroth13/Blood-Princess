#ifndef SHADOWS
#define SHADOWS

#include "../shared_resources.hlsli"

#define SHADOW_MAP_BIAS 0.0005f
#define SHADOW_SAMPLES_DIMENTIONS 6
#define SHADOW_OFFSET_STRENGTH 0.003f
#define DIRECTIONAL_SHADOW_OFFSET_STRENGTH 0.001f

float CalcShadowFactor(
    float3 worldPosition,
    float4x4 lightViewProjMatrix,
    int index,
    const Texture2DArray<float> texArr,
    bool isDirectional = false
)
{
    float offsetStrength;
    
    if (isDirectional)
        offsetStrength = DIRECTIONAL_SHADOW_OFFSET_STRENGTH;
    else
        offsetStrength = SHADOW_OFFSET_STRENGTH;
    
    const static float2 offsets[15] =
    {
        float2(0.9268, 0.3755), float2(0.5276, 0.8495), float2(0.7642, -0.645),
        float2(-0.9908, -0.135), float2(-0.1063, 0.9943), float2(-0.999, 0.0447),
        float2(-0.9766, -0.2149), float2(0.4578, 0.8891), float2(0.5469, -0.8372),
        float2(-0.6466, 0.7629), float2(0.2655, -0.9641), float2(0.3806, 0.9247),
        float2(-0.3756, -0.9268), float2(0.7822, -0.6231), float2(0.6246, -0.781)
    };
    
    float4 lightClipPos = mul(float4(worldPosition, 1.0f), lightViewProjMatrix);
    float3 NDC = lightClipPos.xyz / lightClipPos.w;
    float currentDepth = NDC.z;
    
    float2 uv = float2((NDC.x * 0.5) + 0.5, (-NDC.y * 0.5) + 0.5);
    
    float factor = 0.0f;
    
    uint width = -1;
    uint height = -1;
    uint elements = -1;
    uint nLevels = -1;
    texArr.GetDimensions(index, width, height, elements, nLevels);
    
    uint numSamples = 0;
    
    for (int x = -SHADOW_SAMPLES_DIMENTIONS / 2; x <= SHADOW_SAMPLES_DIMENTIONS / 2; x++)
    {
        for (int y = -SHADOW_SAMPLES_DIMENTIONS / 2; y <= SHADOW_SAMPLES_DIMENTIONS / 2; y++)
        {
            uint offsetIndex = numSamples % 15;

            float2 realOffset = float2(x * (1.0f / width), y * (1.0f / height));
            realOffset += offsets[offsetIndex] * offsetStrength;
            float3 uvc = float3(uv + realOffset, index);
            float closestDepth = texArr.SampleLevel(samplerShadow, uvc, 0);
            if (currentDepth > (closestDepth + SHADOW_MAP_BIAS))
            {
                factor += 0.0f;
            }
            else
            {
                factor += 1.0f;
            }
            numSamples++;
        }
    }
    
    return (factor / numSamples);
}

#endif