#include "../shared_resources.hlsli"
#include "speed_lines.hlsli"

#define ENABLE_TONEMAP 1

RWTexture2D<float4> backBufferUAV;

Texture2D<float4> albedoMetallicTexture : register(t0);
Texture2D<float4> normalRoughnessTexture : register(t1);
Texture2D<float4> depthStencilTexture : register(t2);
Texture2D<float4> outputTexture : register(t3);
Texture2D<float4> noiseTexture : register(t4);

cbuffer DebugView : register(b6)
{
    int debugView;
    float3 pad5;
}

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
#endif // ENABLE_TONEMAP
}

#define MAX_COLORS 128

float ditheringPattern(int2 pixelCoord)
{
    const float pattern[] =
    {
        0.00f, 0.50f, 0.10f, 0.65f,
		0.75f, 0.25f, 0.90f, 0.35f,
		0.20f, 0.70f, 0.05f, 0.50f,
		0.95f, 0.40f, 0.80f, 0.30f
    };
    
    int x = (uint)pixelCoord.x % 4;
    int y = (uint)pixelCoord.y % 4;

    return pattern[y * 4 + x];
}

float reduceColor(float raw, float dither, int depth)
{
    float div = 1.0f / float(depth);
    float val = 0.0f;
    int i = 0.0f;
    
    while (i <= MAX_COLORS)
    {
        if (raw > div * (float(i + 1)))
        {
            i = i + 1;
            continue;
        }
        
        if (raw * float(depth) - float(i) <= dither * 0.999)
        {
            val = div * float(i);
        }
        else
        {
            val = div * float(i + 1);
        }
        return val;

        i = i + 1;
    }
    
    return val;
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
    
    const int debugMode = debugView;
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
    
    float depth = LineariseDepth(depthStencilTexture.SampleLevel(samplerLinearWrap, uv, 0).r) * farPlane;
    float fogStart = 0.0f;
    float fogEnd = 15.0f;
    // S-curve (smoothstep): slow in first 3m, linear between 3m-10m, and smoothly eases into 100% at 15m
    float fogFactor = smoothstep(fogStart, fogEnd, depth);

    const float3 fogColor = float3(0.04f, 0.02f, 0.02f);
    float3 afterFogColour = lerp(ldr, fogColor, fogFactor);
    
    float4 speedLineColour = speedLines(noiseTexture, uv);
    afterFogColour = lerp(afterFogColour, speedLineColour.rgb, speedLineColour.a);
    
    float ditherSize = 2.0f;
    int2 pixelUV = int2(uv.x * width, uv.y * height);
    float ditherAmt = ditheringPattern(pixelUV / ditherSize);
    int colors = MAX_COLORS;
    
    float3 ditherColor = float3(
        reduceColor(afterFogColour.r, (ditherAmt - 0.5) * ditherAmt + 0.5, colors - 1),
        reduceColor(afterFogColour.g, (ditherAmt - 0.5) * ditherAmt + 0.5, colors - 1),
        reduceColor(afterFogColour.b, (ditherAmt - 0.5) * ditherAmt + 0.5, colors - 1)
    );
    
    float3 finalColour = GammaCorrection(ditherColor);

    backBufferUAV[pixel] = float4(finalColour, 1.0f);
}