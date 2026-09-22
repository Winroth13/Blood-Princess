#include "../shared_resources.hlsli"

#define ENABLE_TONEMAP 1

RWTexture2D<float4> backBufferUAV;

Texture2D<float4> albedoMetallicTexture : register(t0);
Texture2D<float4> normalRoughnessTexture : register(t1);
Texture2D<float4> depthStencilTexture : register(t2);
Texture2D<float4> outputTexture : register(t3);

float3 Tonemap(float3 hdr)
{
#if ENABLE_TONEMAP
    const float a = 2.51f;
    const float b = 0.03f;
    const float c = 2.43f;
    const float d = 0.59f;
    const float e = 0.14f;
    return saturate((hdr * (a * hdr + b)) / (hdr * (c * hdr + d) + e));
#else
    return hdr;
#endif
}

float3 GammaCorrection(float3 ldr)
{
    return pow(ldr, 1.0f / 2.2f);
}

float LineariseDepth(float d)
{
    return (nearPlane * farPlane) / (farPlane - d * (farPlane - nearPlane)) / farPlane;
}

float3 SampleQuadrant(float2 uv)
{
    int2 quadrant = floor(uv * 2);
    int index = quadrant.x + 2 * quadrant.y;
    
    float2 localUV = frac(uv * 2);
    
    if (index == 0)
        return albedoMetallicTexture.SampleLevel(samplerLinearWrap, localUV, 0).rgb;

    if (index == 1)
        return normalRoughnessTexture.SampleLevel(samplerLinearWrap, localUV, 0).rgb;
    
    if (index == 2)
        return sqrt(LineariseDepth(depthStencilTexture.SampleLevel(samplerLinearWrap, localUV, 0).r)).rrr;
    
    return Tonemap(outputTexture.SampleLevel(samplerLinearWrap, localUV, 0).rgb);
}

[numthreads(8, 8, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    uint width, height;
    backBufferUAV.GetDimensions(width, height);

    if (id.x >= width || id.y >= height)
        return;
            
    int2 pixel = int2(id.xy);
    float2 positionScreen = float2(pixel) + 0.5f;
    float2 uv = positionScreen / float2(width, height);
    
    float3 ldr = float3(0.0f, 0.0f, 0.0f);
    
    const int debugMode = 0;
    if (debugMode == 1)
        ldr = albedoMetallicTexture.SampleLevel(samplerLinearWrap, uv, 0).rgb;
    else if (debugMode == 2)
        ldr = albedoMetallicTexture.SampleLevel(samplerLinearWrap, uv, 0).aaa;
    else if (debugMode == 3)
        ldr = normalRoughnessTexture.SampleLevel(samplerLinearWrap, uv, 0).rgb;
    else if (debugMode == 4)
        ldr = normalRoughnessTexture.SampleLevel(samplerLinearWrap, uv, 0).aaa;
    else if (debugMode == 5)
        ldr = sqrt(LineariseDepth(depthStencilTexture.SampleLevel(samplerLinearWrap, uv, 0).r)).rrr;
    else if (debugMode == 6)
        ldr = SampleQuadrant(uv);
    else
        ldr = Tonemap(outputTexture.SampleLevel(samplerLinearWrap, uv, 0).rgb);
    
    float3 finalColour = GammaCorrection(ldr);

    backBufferUAV[pixel] = float4(finalColour, 1.0f);
}