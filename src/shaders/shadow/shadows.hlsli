#ifndef SHADOWS
#define SHADOWS

#include "../shared_resources.hlsli"

#define SHADOW_MAP_BIAS 0.0005f
#define SHADOW_SAMPLES_DIMENTIONS 6
#define SHADOW_OFFSET_STRENGTH 0.003f
#define DIRECTIONAL_SHADOW_OFFSET_STRENGTH 0.001f

#define LINEAR_SHADOW_MAP_BIAS 0.00
#define OMNI_SHADOW_SAMPLES 12
#define OMNI_SHADOW_DISK_RADIUS 0.01f
#define OMNI_SHADOW_OFFSET_STRENGTH 0.02f

float CalcOmniShadowFactor(
    float3 fragmentWorldPosition,
    float3 lightPosition,
    int index,
    const TextureCubeArray<float> texArr
)
{
    float3 lightVector = fragmentWorldPosition - lightPosition;
    float currentDepth = length(lightVector);

    float factor = 0.0f;
 
    const float3 sampleOffsetDirections[20] =
    {
        float3(1, 1, 1), float3(1, -1, 1), float3(-1, -1, 1), float3(-1, 1, 1),
        float3(1, 1, -1), float3(1, -1, -1), float3(-1, -1, -1), float3(-1, 1, -1),
        float3(1, 1, 0), float3(1, -1, 0), float3(-1, -1, 0), float3(-1, 1, 0),
        float3(1, 0, 1), float3(-1, 0, 1), float3(1, 0, -1), float3(-1, 0, -1),
        float3(0, 1, 1), float3(0, -1, 1), float3(0, -1, -1), float3(0, 1, -1)
    };
    
    const static float3 sampleOffsets[15] =
    {
        float3(-0.9598, 0.2808, 0.6322), float3(-0.2244, -0.9745, 0.357), float3(-0.9229, 0.3851, -0.9767),
        float3(0.8927, 0.4506, 0.972), float3(0.3507, -0.9365, 0.994), float3(-0.2071, 0.9783, -0.978),
        float3(0.6478, 0.7618, -0.1473), float3(-0.61, 0.7924, 0.4793), float3(-0.3656, -0.9308, 0.376),
        float3(-0.1956, 0.9807, -0.8986), float3(0.8503, -0.5263, 0.7894), float3(-0.8202, -0.5721, -0.1578),
        float3(-0.301, 0.9536, 0.8495), float3(0.912, -0.4101, 0.1386), float3(0.6909, 0.723, 0.3322)
    };
    
    for (uint i = 0; i < OMNI_SHADOW_SAMPLES; ++i)
    {
        float3 offsetLightVector = lightVector + sampleOffsetDirections[i % 20] * OMNI_SHADOW_DISK_RADIUS + sampleOffsets[i % 15] * OMNI_SHADOW_OFFSET_STRENGTH;

        float closestDepth = texArr.SampleLevel(
            samplerShadow,
            float4(normalize(offsetLightVector), index),
            0
        );
        
        if (currentDepth > (closestDepth + LINEAR_SHADOW_MAP_BIAS))
        {
            factor += 0.0f;
        }
        else
        {
            factor += 1.0f;
        }
    }
    
    return (factor / OMNI_SHADOW_SAMPLES);
}

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